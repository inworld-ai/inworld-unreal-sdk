/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldEditorClient.h"
#include "InworldAIEditorModule.h"

#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

#include "InworldStudioTypes.h"

#include "JsonObjectConverter.h"
#include "Interfaces/IPluginManager.h"

#include "HAL/ConsoleManager.h"

#include "Async/Async.h"

namespace Inworld
{
	class FHttpRequest
	{
	public:
		FHttpRequest(const FString& InURL, const FString& InVerb, const FString& InContent, TFunction<void(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)> InCallback)
			: URL(InURL)
			, Verb(InVerb)
			, Content(InContent)
			, Callback(InCallback)
		{
			Process();
		}

		void Cancel();
		bool IsDone() const;

	private:
		void Process();
		void CallCallback(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess);

		FHttpRequestPtr Request;
		FString URL;
		FString Verb;
		FString Content;
		TFunction<void(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)> Callback;
		bool bCallbackCalled = false;
		bool bCanceled = false;
	};

	class FEditorClient
	{
	public:
		void CancelRequests();

		void RequestFirebaseToken(const FInworldEditorClientOptions& Options, TFunction<void(const FString& FirebaseToken)> InCallback);
		void RequestReadyPlayerMeModelData(const FInworldStudioUserCharacterData& CharacterData, TFunction<void(const TArray<uint8>& Data)> InCallback);

		bool IsRequestInProgress() const { return HttpRequests.Num() != 0; }
		const FString& GetError() const { return ErrorMessage; }

	private:
		void CheckDoneRequests();

		void HttpRequest(const FString& InURL, const FString& InVerb, const FString& InContent, TFunction<void(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)> InCallback);

		void Error(FString Message);
		void ClearError();

		TArray<FHttpRequest> HttpRequests;
		
		TFunction<void(const FString& Token)> FirebaseTokenCallback;
		TFunction<void(const TArray<uint8>& Data)> RPMActorCreateCallback;
		FString ServerUrl;

		FString ErrorMessage;
	};
}

void Inworld::FEditorClient::RequestFirebaseToken(const FInworldEditorClientOptions& Options, TFunction<void(const FString& FirebaseToken)> InCallback)
{
	ClearError();

    FString IdToken;
    FString RefreshToken;
    if (!Options.ExchangeToken.Split(":", &IdToken, &RefreshToken))
    {
        Error(FString::Printf(TEXT("EditorClient::RequestFirebaseToken FALURE! Invalid Refresh Token.")));
        return;
    }

	FirebaseTokenCallback = InCallback;
    ServerUrl = Options.ServerUrl;

    FRefreshTokenRequestData RequestData;
    RequestData.grant_type = "refresh_token";
    RequestData.refresh_token = RefreshToken;
    FString JsonString;
    FJsonObjectConverter::UStructToJsonObjectString(RequestData, JsonString);

    HttpRequest("https://securetoken.googleapis.com/v1/token?key=AIzaSyAPVBLVid0xPwjuU4Gmn_6_GyqxBq-SwQs", "POST", JsonString,
        [this](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess) 
        {
			CheckDoneRequests();

			if (!bSuccess)
			{
				Error(FString::Printf(TEXT("EditorClient::OnFirebaseTokenResponse FALURE! Code: %d"), Response ? Response->GetResponseCode() : -1));
				return;
			}

			FRefreshTokenResponseData ResponseData;
			if (!FJsonObjectConverter::JsonObjectStringToUStruct(Response->GetContentAsString(), &ResponseData))
			{
				Error(FString::Printf(TEXT("EditorClient::OnFirebaseTokenResponse FALURE! Invalid Response Data.")));
				return;
			}

			if (!ensure(FirebaseTokenCallback))
			{
				return;
			}

			FirebaseTokenCallback(ResponseData.id_token);
        }
	);
}

void Inworld::FEditorClient::CancelRequests()
{
	for (auto& Request : HttpRequests)
	{
		Request.Cancel();
	}

	HttpRequests.Empty();
}

void Inworld::FEditorClient::CheckDoneRequests()
{
	AsyncTask(ENamedThreads::GameThread, [this]()
		{
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
	);
}

void Inworld::FEditorClient::HttpRequest(const FString& InURL, const FString& InVerb, const FString& InContent, TFunction<void(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)> InCallback)
{
    HttpRequests.Emplace(InURL, InVerb, InContent, InCallback);
}

void Inworld::FEditorClient::RequestReadyPlayerMeModelData(const FInworldStudioUserCharacterData& CharacterData, TFunction<void(const TArray<uint8>& Data)> InCallback)
{
    RPMActorCreateCallback = InCallback;

    static const FString UriArgsStr = "?pose=A&meshLod=0&textureAtlas=none&textureSizeLimit=1024&morphTargets=ARKit,Oculus%20Visemes&useHands=true";
    HttpRequest(FString::Printf(TEXT("%s%s"), *CharacterData.RpmModelUri, *UriArgsStr), "GET", FString(), [this, &CharacterData](FHttpRequestPtr InRequest, FHttpResponsePtr InResponse, bool bSuccess)
        {
			CheckDoneRequests();

            if (!bSuccess || InResponse->GetContent().Num() == 0)
			{
                Error(FString::Printf(TEXT("EditorClient::RequestReadyPlayerMeModelData request FALURE!, Code: %d"), InResponse ? InResponse->GetResponseCode() : -1));
                return;
            }

            if (!ensure(RPMActorCreateCallback))
            {
                return;
            }

            RPMActorCreateCallback(InResponse->GetContent());
            });
}

void Inworld::FEditorClient::Error(FString Message)
{
	UE_LOG(LogInworldAIEditor, Error, TEXT("%s"), *Message);
    ErrorMessage = Message;
	CancelRequests();
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

void FInworldEditorClient::Init()
{
	InworldEditorClient = MakeShared<Inworld::FEditorClient>();
}

void FInworldEditorClient::Destroy()
{
	if (InworldEditorClient)
	{
		InworldEditorClient->CancelRequests();
	}
	InworldEditorClient.Reset();
}

void FInworldEditorClient::RequestFirebaseToken(const FInworldEditorClientOptions& Options, TFunction<void(const FString& FirebaseToken)> InCallback)
{
	InworldEditorClient->RequestFirebaseToken(Options, InCallback);
}

void FInworldEditorClient::RequestReadyPlayerMeModelData(const FInworldStudioUserCharacterData& CharacterData, TFunction<void(const TArray<uint8>& Data)> InCallback)
{
	InworldEditorClient->RequestReadyPlayerMeModelData(CharacterData, InCallback);
}

void FInworldEditorClient::CancelRequests()
{
	InworldEditorClient->CancelRequests();
}

bool FInworldEditorClient::IsRequestInProgress() const
{
	return InworldEditorClient->IsRequestInProgress();
}

const FString& FInworldEditorClient::GetError() const
{
	return InworldEditorClient->GetError();
}
