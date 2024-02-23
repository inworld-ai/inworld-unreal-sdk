/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldStudio.h"
#include "InworldAIClientModule.h"
#include "InworldAsyncRoutine.h"

#include "Async/Async.h"
#include "Async/TaskGraphInterfaces.h"

THIRD_PARTY_INCLUDES_START
// UNREAL ENGINE 4
#pragma warning(push)
#pragma warning(disable:4583)
#pragma warning(disable:4582)
#include "GrpcHelpers.h"
#pragma warning(pop)
// UNREAL ENGINE 4
#include "Service.h"
THIRD_PARTY_INCLUDES_END

#include <string>

namespace Inworld
{
	class FStudio
	{
	public:
		void RequestStudioUserData(const FString& Token, const FString& ServerUrl, TFunction<void(bool bSuccess)> InCallback);

		void CancelRequests();
		bool IsRequestInProgress() const { return Requests.Num() != 0; }

		const FString& GetError() const { return ErrorMessage; }
		FInworldStudioUserData GetStudioUserData() const { return StudioUserData; }

	private:
		TFunction<void(bool bSuccess)> Callback;
		FString ServerUrl;
		FString InworldToken;
		FInworldStudioUserData StudioUserData;

		FCriticalSection RequestsMutex;
		TArray<FInworldAsyncRoutine> Requests;

		FString ErrorMessage;

	public:
		TWeakPtr<FStudio> SelfWeakPtr;

	private:
		void Request(const std::string& ThreadName, std::unique_ptr<Inworld::Runnable> Runnable);
		void CheckDoneRequests();

		void OnGenerateUserToken(const InworldV1alpha::GenerateTokenUserResponse& Response);
		void OnWorkspacesReady(const InworldV1alpha::ListWorkspacesResponse& Response);
		void OnApiKeysReady(const InworldV1alpha::ListApiKeysResponse& Response, FInworldStudioUserWorkspaceData& Workspace);
		void OnScenesReady(const InworldV1alpha::ListScenesResponse& Response, FInworldStudioUserWorkspaceData& Workspace);
		void OnCharactersReady(const InworldV1alpha::ListCharactersResponse& Response, FInworldStudioUserWorkspaceData& Workspace);

		void Error(FString Error);
		void ClearError();
	};
}

FInworldStudio::FInworldStudio()
{
	InworldStudio = MakeShared<Inworld::FStudio>();
	InworldStudio->SelfWeakPtr = InworldStudio;
}

FInworldStudio::~FInworldStudio()
{
	InworldStudio.Reset();
}

void FInworldStudio::CancelRequests()
{
	InworldStudio->CancelRequests();
}

bool FInworldStudio::IsRequestInProgress() const
{
	return InworldStudio->IsRequestInProgress();
}

void FInworldStudio::RequestStudioUserData(const FString& Token, const FString& ServerUrl, TFunction<void(bool bSuccess)> InCallback)
{
	InworldStudio->RequestStudioUserData(Token, ServerUrl, InCallback);
}

const FString& FInworldStudio::GetError() const
{
	return InworldStudio->GetError();
}

FInworldStudioUserData FInworldStudio::GetStudioUserData() const
{
	return InworldStudio->GetStudioUserData();
}

void Inworld::FStudio::RequestStudioUserData(const FString& InToken, const FString& InServerUrl, TFunction<void(bool bSuccess)> InCallback)
{
	ClearError();

	ServerUrl = InServerUrl;
	Callback = InCallback;

	Request(
		"RunnableGenerateUserTokenRequest",
		std::make_unique<Inworld::RunnableGenerateUserTokenRequest>(
			TCHAR_TO_UTF8(*InToken),
			TCHAR_TO_UTF8(*InServerUrl),
			[this](const grpc::Status& Status, const InworldV1alpha::GenerateTokenUserResponse& Response)
			{
				if (!Status.ok())
				{
					Error(FString::Printf(TEXT("FRunnableGenerateUserTokenRequest FALURE! %s, Code: %d"), UTF8_TO_TCHAR(Status.error_message().c_str()), Status.error_code()));
					return;
				}

				OnGenerateUserToken(Response);
			}
		)
	);
}

void Inworld::FStudio::OnGenerateUserToken(const InworldV1alpha::GenerateTokenUserResponse& Response)
{
	InworldToken = UTF8_TO_TCHAR(Response.token().c_str());
	StudioUserData.Workspaces.Empty();

	Request(
		"RunnableListWorkspacesRequest",
		std::make_unique<Inworld::RunnableListWorkspacesRequest>(
			TCHAR_TO_UTF8(*InworldToken),
			TCHAR_TO_UTF8(*ServerUrl),
			[this](const grpc::Status& Status, const InworldV1alpha::ListWorkspacesResponse& Response)
			{
				if (!Status.ok())
				{
					Error(FString::Printf(TEXT("FRunnableListWorkspacesRequest FALURE! %s, Code: %d"), UTF8_TO_TCHAR(Status.error_message().c_str()), Status.error_code()));
					return;
				}

				OnWorkspacesReady(Response);
				CheckDoneRequests();
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

void Inworld::FStudio::OnWorkspacesReady(const InworldV1alpha::ListWorkspacesResponse& Response)
{
	StudioUserData.Workspaces.Reserve(Response.workspaces_size());

	for (int32 i = 0; i < Response.workspaces_size(); i++)
	{
		const auto& GrpcWorkspace = Response.workspaces(i);
		auto& Workspace = StudioUserData.Workspaces.Emplace_GetRef();
		Workspace.Name = UTF8_TO_TCHAR(GrpcWorkspace.name().data());
		Workspace.ShortName = CreateShortName(Workspace.Name);

		Request(
			"RunnableListScenesRequest",
			std::make_unique<Inworld::RunnableListScenesRequest>(
				TCHAR_TO_UTF8(*InworldToken),
				TCHAR_TO_UTF8(*ServerUrl),
				GrpcWorkspace.name(),
				[this, &Workspace](const grpc::Status& Status, const InworldV1alpha::ListScenesResponse& Response)
				{
					if (!Status.ok())
					{
						Error(FString::Printf(TEXT("FRunnableListScenesRequest FALURE! %s, Code: %d"), UTF8_TO_TCHAR(Status.error_message().c_str()), Status.error_code()));
						return;
					}

					OnScenesReady(Response, Workspace);
					CheckDoneRequests();
				}
			)
		);

		Request(
			"RunnableListCharactersRequest",
			std::make_unique<Inworld::RunnableListCharactersRequest>(
				TCHAR_TO_UTF8(*InworldToken),
				TCHAR_TO_UTF8(*ServerUrl),
				GrpcWorkspace.name(),
				[this, &Workspace](const grpc::Status& Status, const InworldV1alpha::ListCharactersResponse& Response)
				{
					if (!Status.ok())
					{
						Error(FString::Printf(TEXT("FRunnableListCharactersRequest FALURE! %s, Code: %d"), UTF8_TO_TCHAR(Status.error_message().c_str()), Status.error_code()));
						return;
					}

					OnCharactersReady(Response, Workspace);
					CheckDoneRequests();
				}
			)
		);

		Request(
			"RunnableListApiKeysRequest",
			std::make_unique<Inworld::RunnableListApiKeysRequest>(
				TCHAR_TO_UTF8(*InworldToken),
				TCHAR_TO_UTF8(*ServerUrl),
				GrpcWorkspace.name(),
				[this, &Workspace](const grpc::Status& Status, const InworldV1alpha::ListApiKeysResponse& Response)
				{
					if (!Status.ok())
					{
						Error(FString::Printf(TEXT("FRunnableListCharactersRequest FALURE! %s, Code: %d"), UTF8_TO_TCHAR(Status.error_message().c_str()), Status.error_code()));
						return;
					}

					OnApiKeysReady(Response, Workspace);
					CheckDoneRequests();
				}
			)
		);
	}
}

void Inworld::FStudio::OnApiKeysReady(const InworldV1alpha::ListApiKeysResponse& Response, FInworldStudioUserWorkspaceData& Workspace)
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

void Inworld::FStudio::OnScenesReady(const InworldV1alpha::ListScenesResponse& Response, FInworldStudioUserWorkspaceData& Workspace)
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

void Inworld::FStudio::OnCharactersReady(const InworldV1alpha::ListCharactersResponse& Response, FInworldStudioUserWorkspaceData& Workspace)
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

void Inworld::FStudio::CancelRequests()
{
	FScopeLock Lock(&RequestsMutex);

	for (auto& Request : Requests)
	{
		Request.Stop();
	}

	Requests.Empty();
}

void Inworld::FStudio::Request(const std::string& ThreadName, std::unique_ptr<Inworld::Runnable> Runnable)
{
	FScopeLock Lock(&RequestsMutex);

	auto& AsyncTask = Requests.Emplace_GetRef();
	AsyncTask.Start(ThreadName, std::move(Runnable));
}

void Inworld::FStudio::CheckDoneRequests()
{
	AsyncTask(ENamedThreads::GameThread, [this]()
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

			if (Requests.Num() == 0)
			{
				Callback(true);
			}
		}
	);
}

void Inworld::FStudio::Error(FString Message)
{
	UE_LOG(LogInworldAIClient, Error, TEXT("%s"), *Message);
	ErrorMessage = Message;
	CancelRequests();
	Callback(false);
}

void Inworld::FStudio::ClearError()
{
	ErrorMessage.Empty();
}
