/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldEditorClient.h"

#include "JsonObjectConverter.h"
#include "Interfaces/IPluginManager.h"

#include "HAL/ConsoleManager.h"

#include "NDK/GrpcHelpers.h"
#include "NDK/Utils/Log.h"
#include "NDK/Utils/Utils.h"

#include "InworldStudioUserData.h"

void Inworld::FEditorClient::RequestUserData(const FEditorClientOptions& Options, TFunction<void(const FInworldStudioUserData& UserData, bool IsError)> InCallback)
{
	ClearError();

    FString IdToken;
    FString RefreshToken;
    if (!Options.ExchangeToken.Split(":", &IdToken, &RefreshToken))
    {
        Error(FString::Printf(TEXT("FEditorClient::RequestUserData FALURE! Invalid Refresh Token.")));
        return;
    }

    UserData.Workspaces.Empty();

	UserDataCallback = InCallback;
    ServerUrl = TCHAR_TO_UTF8(*Options.ServerUrl);

    FRefreshTokenRequestData RequestData;
    RequestData.grant_type = "refresh_token";
    RequestData.refresh_token = RefreshToken;
    FString JsonString;
    FJsonObjectConverter::UStructToJsonObjectString(RequestData, JsonString);

    HttpRequest("https://securetoken.googleapis.com/v1/token?key=AIzaSyAPVBLVid0xPwjuU4Gmn_6_GyqxBq-SwQs", "POST", JsonString,
        [this](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess) 
        { 
            OnFirebaseTokenResponse(Request, Response, bSuccess); 
        });
}

void Inworld::FEditorClient::CancelRequests()
{
	for (auto& Request : HttpRequests)
	{
		Request.Cancel();
	}

	HttpRequests.Empty();

    {
        FScopeLock Lock(&RequestsMutex);

        for (auto& Request : Requests)
        {
            Request.Stop();
        }

        Requests.Empty();
    }

    UserDataCallback = nullptr;
}

void Inworld::FEditorClient::Request(std::string ThreadName, std::unique_ptr<Inworld::Runnable> InRunnable)
{
	FScopeLock Lock(&RequestsMutex);

	auto& AsyncTask = Requests.Emplace_GetRef();
	AsyncTask.Start(ThreadName, std::move(InRunnable));
}

void Inworld::FEditorClient::CheckDoneRequests()
{
	if (Requests.Num() == 0 && HttpRequests.Num() == 0)
	{
		if (UserDataCallback)
		{
			UserDataCallback(UserData, !ErrorMessage.IsEmpty());
			UserDataCallback = nullptr;
		}
        return;
	}

    {
        FScopeLock Lock(&RequestsMutex);

        for (int32 i = 0; i < Requests.Num();)
        {
            auto& Request = Requests[i];
            if (Request.IsDone())
            {
                Requests.RemoveAt(i, 1, false);
            }
            else
            {
                i++;
            }
        }
	}

	for (int32 i = 0; i < HttpRequests.Num();)
	{
		auto& Request = HttpRequests[i];
		if (Request.IsDone())
		{
            HttpRequests.RemoveAt(i, 1, false);
		}
		else
		{
			i++;
		}
	}
}

void Inworld::FEditorClient::HttpRequest(const FString& InURL, const FString& InVerb, const FString& InContent, TFunction<void(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)> InCallback)
{
    HttpRequests.Emplace(InURL, InVerb, InContent, InCallback);
}

void Inworld::FEditorClient::OnFirebaseTokenResponse(FHttpRequestPtr InRequest, FHttpResponsePtr InResponse, bool bSuccess)
{
    if (!bSuccess)
    {
		Error(FString::Printf(TEXT("FEditorClient::OnFirebaseTokenResponse FALURE! Code: %d"), InResponse ? InResponse->GetResponseCode() : -1));
		return;
    }

    FRefreshTokenResponseData ResponseData;
    if (!FJsonObjectConverter::JsonObjectStringToUStruct(InResponse->GetContentAsString(), &ResponseData))
    {
        Error(FString::Printf(TEXT("FEditorClient::OnFirebaseTokenResponse FALURE! Invalid Response Data.")));
		return;
    }

	Request(
        "RunnableGenerateUserTokenRequest",
		MakeRunnableGenerateUserTokenRequest(
			TCHAR_TO_UTF8(*ResponseData.id_token),
			ServerUrl,
			[this](const grpc::Status& Status, const InworldV1alpha::GenerateTokenUserResponse& Response)
			{
				if (!Status.ok())
				{
					Error(FString::Printf(TEXT("FRunnableGenerateUserTokenRequest FALURE! %s, Code: %d"), UTF8_TO_TCHAR(Status.error_message().c_str()), Status.error_code()));
					return;
				}

				OnUserTokenReady(Response);
			}
        )
    );
}

void Inworld::FEditorClient::Tick(float DeltaTime)
{
    if (!ErrorMessage.IsEmpty())
    {
        CancelRequests();
    }

	CheckDoneRequests();
}

void Inworld::FEditorClient::RequestReadyPlayerMeModelData(const FInworldStudioUserCharacterData& CharacterData, TFunction<void(const TArray<uint8>& Data)> InCallback)
{
    RPMActorCreateCallback = InCallback;

    static const FString UriArgsStr = "?pose=A&meshLod=0&textureAtlas=none&textureSizeLimit=1024&morphTargets=ARKit,Oculus%20Visemes&useHands=true";
    HttpRequest(FString::Printf(TEXT("%s%s"), *CharacterData.RpmModelUri, *UriArgsStr), "GET", FString(), [this, &CharacterData](FHttpRequestPtr InRequest, FHttpResponsePtr InResponse, bool bSuccess)
        {
            if (!bSuccess || InResponse->GetContent().Num() == 0)
			{
                Error(FString::Printf(TEXT("FEditorClient::RequestReadyPlayerMeModelData request FALURE!, Code: %d"), InResponse ? InResponse->GetResponseCode() : -1));
                return;
            }

            if (!ensure(RPMActorCreateCallback))
            {
                return;
            }

            RPMActorCreateCallback(InResponse->GetContent());
            });
}

bool Inworld::FEditorClient::GetActiveApiKey(FInworldStudioUserWorkspaceData& InWorkspaceData, FInworldStudioUserApiKeyData& InApiKeyData)
{
    for (auto& ApiKeyData : InWorkspaceData.ApiKeys)
    {
        if (ApiKeyData.IsActive)
        {
            InApiKeyData = ApiKeyData;
            return true;
        }
    }
    return false;
}

void Inworld::FEditorClient::OnUserTokenReady(const InworldV1alpha::GenerateTokenUserResponse& Response)
{
    InworldToken = Response.token();

	Request(
        "RunnableListWorkspacesRequest",
        MakeRunnableListWorkspacesRequest(
			InworldToken,
			ServerUrl,
			[this](const grpc::Status& Status, const InworldV1alpha::ListWorkspacesResponse& Response)
			{
				if (!Status.ok())
				{
					Error(FString::Printf(TEXT("FRunnableListWorkspacesRequest FALURE! %s, Code: %d"), UTF8_TO_TCHAR(Status.error_message().c_str()), Status.error_code()));
					return;
				}

				OnWorkspacesReady(Response);
			}
        )
    );
}

static FString CreateShortName(const FString& Name)
{
	int32 Idx;
    FString ShortName = Name;
	if (ShortName.FindLastChar('/', Idx))
	{
        ShortName.RightChopInline(Idx + 1);
	}
    return MoveTemp(ShortName);
}

void Inworld::FEditorClient::OnWorkspacesReady(const InworldV1alpha::ListWorkspacesResponse& Response)
{
    UserData.Workspaces.Reserve(Response.workspaces_size());

    for (int32 i = 0; i < Response.workspaces_size(); i++)
    {
        const auto& GrpcWorkspace = Response.workspaces(i);
        auto& Workspace = UserData.Workspaces.Emplace_GetRef();
        Workspace.Name = UTF8_TO_TCHAR(GrpcWorkspace.name().data());
        Workspace.ShortName = CreateShortName(Workspace.Name);

		Request(
            "RunnableListScenesRequest",
            MakeRunnableListScenesRequest(
				InworldToken,
				ServerUrl,
				GrpcWorkspace.name(),
				[this, &Workspace](const grpc::Status& Status, const InworldV1alpha::ListScenesResponse& Response)
				{
					if (!Status.ok())
					{
						Error(FString::Printf(TEXT("FRunnableListScenesRequest FALURE! %s, Code: %d"), UTF8_TO_TCHAR(Status.error_message().c_str()), Status.error_code()));
						return;
					}

					OnScenesReady(Response, Workspace);
				}
            )
        );

		Request(
            "RunnableListCharactersRequest",
			MakeRunnableListCharactersRequest(
				InworldToken,
				ServerUrl,
				GrpcWorkspace.name(),
				[this, &Workspace](const grpc::Status& Status, const InworldV1alpha::ListCharactersResponse& Response)
				{
					if (!Status.ok())
					{
						Error(FString::Printf(TEXT("FRunnableListCharactersRequest FALURE! %s, Code: %d"), UTF8_TO_TCHAR(Status.error_message().c_str()), Status.error_code()));
						return;
					}

					OnCharactersReady(Response, Workspace);
				}
            )
        );

		Request(
            "RunnableListApiKeysRequest",
            MakeRunnableListApiKeysRequest(
				InworldToken,
				ServerUrl,
				GrpcWorkspace.name(),
				[this, &Workspace](const grpc::Status& Status, const InworldV1alpha::ListApiKeysResponse& Response)
				{
					if (!Status.ok())
					{
						Error(FString::Printf(TEXT("FRunnableListCharactersRequest FALURE! %s, Code: %d"), UTF8_TO_TCHAR(Status.error_message().c_str()), Status.error_code()));
						return;
					}

					OnApiKeysReady(Response, Workspace);
				}
            )
        );
    }
}

void Inworld::FEditorClient::OnApiKeysReady(const InworldV1alpha::ListApiKeysResponse& Response, FInworldStudioUserWorkspaceData& Workspace)
{
    Workspace.ApiKeys.Reserve(Response.api_keys_size());

    for (int32 i = 0; i < Response.api_keys_size(); i++)
    {
        const auto& GrpcApiKey = Response.api_keys(i);
        auto& ApiKey = Workspace.ApiKeys.Emplace_GetRef();
        ApiKey.Name = UTF8_TO_TCHAR(GrpcApiKey.name().data());
        ApiKey.Key = UTF8_TO_TCHAR(GrpcApiKey.key().data());
        ApiKey.Secret = UTF8_TO_TCHAR(GrpcApiKey.secret().data());
        ApiKey.IsActive = GrpcApiKey.state() == InworldV1alpha::ApiKey_State_ACTIVE;
    }
}

void Inworld::FEditorClient::OnScenesReady(const InworldV1alpha::ListScenesResponse& Response, FInworldStudioUserWorkspaceData& Workspace)
{
	Workspace.Scenes.Reserve(Response.scenes_size());

	for (int32 i = 0; i < Response.scenes_size(); i++)
	{
		const auto& GrpcScene = Response.scenes(i);
		auto& Scene = Workspace.Scenes.Emplace_GetRef();
		Scene.Name = UTF8_TO_TCHAR(GrpcScene.name().data());
        Scene.ShortName = CreateShortName(Scene.Name);
        Scene.Characters.Reserve(GrpcScene.character_references_size());
        for (int32 j = 0; j < GrpcScene.character_references_size(); j++)
        {
            Scene.Characters.Emplace(UTF8_TO_TCHAR(GrpcScene.character_references(j).character().data()));
        }
	}
}

void Inworld::FEditorClient::OnCharactersReady(const InworldV1alpha::ListCharactersResponse& Response, FInworldStudioUserWorkspaceData& Workspace)
{
	Workspace.Characters.Reserve(Response.characters_size());

	for (int32 i = 0; i < Response.characters_size(); i++)
	{
		const auto& GrpcCharacter = Response.characters(i);
		auto& Character = Workspace.Characters.Emplace_GetRef();
		Inworld::GrpcHelper::CharacterInfo CharInfo = Inworld::GrpcHelper::CreateCharacterInfo(GrpcCharacter);
		Character.Name = UTF8_TO_TCHAR(CharInfo._Name.c_str());
        Character.ShortName = CreateShortName(Character.Name);
        Character.RpmModelUri = UTF8_TO_TCHAR(CharInfo._RpmModelUri.c_str());
		Character.RpmImageUri = UTF8_TO_TCHAR(CharInfo._RpmImageUri.c_str());
        Character.RpmPortraitUri = UTF8_TO_TCHAR(CharInfo._RpmPortraitUri.c_str());
        Character.RpmPostureUri = UTF8_TO_TCHAR(CharInfo._RpmPostureUri.c_str());
        Character.bMale = CharInfo._bMale;
	}
}

void Inworld::FEditorClient::OnCharacterModelReady(FHttpResponsePtr Response, FInworldStudioUserCharacterData& CharacterData)
{

}

void Inworld::FEditorClient::OnCharacterImageReady(FHttpResponsePtr Response, FInworldStudioUserCharacterData& CharacterData)
{

}

void Inworld::FEditorClient::OnCharacterPortraitReady(FHttpResponsePtr Response, FInworldStudioUserCharacterData& CharacterData)
{

}

void Inworld::FEditorClient::OnCharacterPostureReady(FHttpResponsePtr Response, FInworldStudioUserCharacterData& CharacterData)
{

}

void Inworld::FEditorClient::Error(FString Message)
{
    Inworld::LogError(TCHAR_TO_UTF8(*Message));
    ErrorMessage = Message;
}

void Inworld::FEditorClient::ClearError()
{
	ErrorMessage.Empty();
}

void Inworld::FHttpRequest::Process()
{
    if (Request.IsValid())
    {
        Request->CancelRequest();
    }
    Request.Reset();
	Request = FHttpModule::Get().CreateRequest();

	Request->SetURL(URL);
	Request->SetVerb(Verb);
	if (!Content.IsEmpty())
	{
		Request->SetContentAsString(Content);
		Request->SetHeader("Content-Type", "application/json");
	}
	Request->OnProcessRequestComplete().BindRaw(this, &FHttpRequest::CallCallback);
	Request->SetTimeout(5.f);
	Request->ProcessRequest();
}

void Inworld::FHttpRequest::Cancel()
{
	bCanceled = true;
	if (Request.IsValid())
	{
		Request->CancelRequest();
	}
}

bool Inworld::FHttpRequest::IsDone() const
{
	if (bCanceled || bCallbackCalled)
	{
		return true;
	}

	return false;
}

void Inworld::FHttpRequest::CallCallback(FHttpRequestPtr RequestPtr, FHttpResponsePtr ResponsePtr, bool bSuccess)
{
	if (!bCanceled && ensure(Callback))
	{
		Callback(RequestPtr, ResponsePtr, bSuccess);
		bCallbackCalled = true;
	}
}
