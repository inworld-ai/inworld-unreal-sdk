/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include <string>
#include <memory>
#include <functional>

#include "Http.h"
#include "NDK/RunnableCommand.h"
#include "InworldRunnable.h"

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

	struct FEditorClientOptions
	{
		FString ServerUrl;
		FString ExchangeToken;
	};

	class INWORLDAIEDITOR_API FEditorClient
	{
	public:
		void RequestUserData(const FEditorClientOptions& Options, TFunction<void(const FInworldStudioUserData& UserData, bool IsError)> InCallback);
		void CancelRequests();

		bool IsUserDataReady() const { return UserData.Workspaces.Num() != 0; }
		bool IsRequestInProgress() const { return Requests.Num() != 0 || HttpRequests.Num() != 0; }
		const FInworldStudioUserData& GetUserData() const { return UserData; }

		void Tick(float DeltaTime);

		void RequestReadyPlayerMeModelData(const FInworldStudioUserCharacterData& CharacterData, TFunction<void(const TArray<uint8>& Data)> InCallback);

		bool GetActiveApiKey(FInworldStudioUserWorkspaceData& WorkspaceData, FInworldStudioUserApiKeyData& ApiKeyData);

		const FString& GetError() const { return ErrorMessage; }

	private:

		void Request(std::string ThreadName, std::unique_ptr<Inworld::Runnable> InRunnable);
		
		void CheckDoneRequests();

		void HttpRequest(const FString& InURL, const FString& InVerb, const FString& InContent, TFunction<void(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)> InCallback);

		void OnFirebaseTokenResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess);

		void OnUserTokenReady(const InworldV1alpha::GenerateTokenUserResponse& Response);
		void OnWorkspacesReady(const InworldV1alpha::ListWorkspacesResponse& Response);
		void OnApiKeysReady(const InworldV1alpha::ListApiKeysResponse& Response, FInworldStudioUserWorkspaceData& WorkspaceData);
		void OnScenesReady(const InworldV1alpha::ListScenesResponse& Response, FInworldStudioUserWorkspaceData& WorkspaceData);
		void OnCharactersReady(const InworldV1alpha::ListCharactersResponse& Response, FInworldStudioUserWorkspaceData& WorkspaceData);
		
		void OnCharacterModelReady(FHttpResponsePtr Response, FInworldStudioUserCharacterData& CharacterData);
		void OnCharacterImageReady(FHttpResponsePtr Response, FInworldStudioUserCharacterData& CharacterData);
		void OnCharacterPortraitReady(FHttpResponsePtr Response, FInworldStudioUserCharacterData& CharacterData);
		void OnCharacterPostureReady(FHttpResponsePtr Response, FInworldStudioUserCharacterData& CharacterData);

		void Error(FString Message);
		void ClearError();

		FCriticalSection RequestsMutex;
		TArray<FAsyncRoutine> Requests;

		TArray<FHttpRequest> HttpRequests;

		FInworldStudioUserData UserData;

		std::string InworldToken;
		
		TFunction<void(const FInworldStudioUserData& UserData, bool IsError)> UserDataCallback;
		TFunction<void(const TArray<uint8>& Data)> RPMActorCreateCallback;
		std::string ServerUrl;

		FString ErrorMessage;
	};
}
