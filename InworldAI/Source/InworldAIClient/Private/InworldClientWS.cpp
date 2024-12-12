/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */
#ifdef INWORLD_WITH_WS

#include "InworldClientWS.h"
#include "InworldAIClientModule.h"
#include "InworldAIClientSettings.h"

#define UI UI_ST
THIRD_PARTY_INCLUDES_START
#include "Ssl.h"
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <openssl/ssl.h>
THIRD_PARTY_INCLUDES_END
#undef UI

#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "JsonObjectConverter.h"

#include "WebSocketsModule.h"
#include "IWebSocket.h"

#define NOT_IMPLEMENTED_RETURN(Return) UE_LOG(LogInworldAIClient, Warning, TEXT("InworldClientWS::%s skipped: due to lack of WebSocket support."), *FString(__func__)); return Return;

namespace Security
{
	struct FSHA256
	{
		using ByteArray = uint8[32];
		FSHA256() = default;
		void ToString(FAnsiStringBuilderBase& Sb) const
		{
			UE::String::BytesToHexLower(Hash, Sb);
		}
		alignas(uint32) ByteArray Hash {};
	};
	FSHA256 Sha256(const uint8* Input, size_t InputLen)
	{
		FSHA256 Output;
		SHA256(Input, InputLen, Output.Hash);
		return Output;
	}
	FSHA256 HmacSha256(const uint8* Input, size_t InputLen, const uint8* Key, size_t KeyLen)
	{
		FSHA256 Output;
		unsigned int OutputLen = 0;
		HMAC(EVP_sha256(), Key, KeyLen, (const unsigned char*)Input, InputLen, Output.Hash, &OutputLen);
		return Output;
	}
	FSHA256 HmacSha256(const char* Input, const uint8* Key, size_t KeyLen)
	{
		return HmacSha256((const uint8*)Input, (size_t)FCStringAnsi::Strlen(Input), Key, KeyLen);
	}
} // namespace Security

InworldClientWS::InworldClientWS()
{

}

InworldClientWS::~InworldClientWS()
{
	TokenRequest.Reset();
}

void InworldClientWS::StartSessionFromScene(const FInworldScene& Scene, const FInworldPlayerProfile& PlayerProfile, const FInworldCapabilitySet& CapabilitySet, const TMap<FString, FString>& Metadata, const FString& WorkspaceOverride, const FInworldAuth& AuthOverride)
{
	FOnTokenGenerated Callback;
	Callback.BindLambda([this](bool bTokenValid)
		{
			// Start Session w/ token
		}
	);
	GenerateToken(Callback, WorkspaceOverride, AuthOverride);
}

void InworldClientWS::StartSessionFromSave(const FInworldSave& Save, const FInworldPlayerProfile& PlayerProfile, const FInworldCapabilitySet& CapabilitySet, const TMap<FString, FString>& Metadata, const FString& WorkspaceOverride, const FInworldAuth& AuthOverride)
{
	NOT_IMPLEMENTED_RETURN(void())
}

void InworldClientWS::StartSessionFromToken(const FInworldToken& Token, const FInworldPlayerProfile& PlayerProfile, const FInworldCapabilitySet& CapabilitySet, const TMap<FString, FString>& Metadata, const FString& WorkspaceOverride, const FInworldAuth& AuthOverride)
{
	NOT_IMPLEMENTED_RETURN(void())
}

void InworldClientWS::StopSession()
{
	NOT_IMPLEMENTED_RETURN(void())
}

void InworldClientWS::PauseSession()
{
	NOT_IMPLEMENTED_RETURN(void())
}

void InworldClientWS::ResumeSession()
{
	NOT_IMPLEMENTED_RETURN(void())
}

FInworldToken InworldClientWS::GetSessionToken() const
{
	NOT_IMPLEMENTED_RETURN({})
}

void InworldClientWS::LoadPlayerProfile(const FInworldPlayerProfile& PlayerProfile)
{
	NOT_IMPLEMENTED_RETURN(void())
}

FInworldCapabilitySet InworldClientWS::GetCapabilities() const
{
	NOT_IMPLEMENTED_RETURN({})
}

void InworldClientWS::LoadCapabilities(const FInworldCapabilitySet& CapabilitySet)
{
	NOT_IMPLEMENTED_RETURN(void())
}

void InworldClientWS::SaveSession(FOnInworldSessionSavedCallback Callback)
{
	NOT_IMPLEMENTED_RETURN(void())
}

void InworldClientWS::SendInteractionFeedback(const FString& InteractionId, bool bIsLike, const FString& Message)
{
	NOT_IMPLEMENTED_RETURN(void())
}

void InworldClientWS::LoadCharacter(const FString& Id)
{
	NOT_IMPLEMENTED_RETURN(void())
}

void InworldClientWS::LoadCharacters(const TArray<FString>& Ids)
{
	NOT_IMPLEMENTED_RETURN(void())
}

void InworldClientWS::UnloadCharacter(const FString& Id)
{
	NOT_IMPLEMENTED_RETURN(void())
}

void InworldClientWS::UnloadCharacters(const TArray<FString>& Ids)
{
	NOT_IMPLEMENTED_RETURN(void())
}

FString InworldClientWS::UpdateConversation(const FString& ConversationId, const TArray<FString>& AgentIds, bool bIncludePlayer)
{
	NOT_IMPLEMENTED_RETURN({})
}

FInworldWrappedPacket InworldClientWS::SendTextMessage(const FString& AgentId, const FString& Text)
{
	NOT_IMPLEMENTED_RETURN({})
}

FInworldWrappedPacket InworldClientWS::SendTextMessageToConversation(const FString& ConversationId, const FString& Text)
{
	NOT_IMPLEMENTED_RETURN({})
}

void InworldClientWS::InitSpeechProcessor(EInworldPlayerSpeechMode Mode, const FInworldPlayerSpeechOptions& SpeechOptions)
{
	NOT_IMPLEMENTED_RETURN(void())
}

void InworldClientWS::DestroySpeechProcessor()
{
	NOT_IMPLEMENTED_RETURN(void())
}

void InworldClientWS::SendSoundMessage(const FString& AgentId, const TArray<uint8>& InputData, const TArray<uint8>& OutputData)
{
	NOT_IMPLEMENTED_RETURN(void())
}

void InworldClientWS::SendSoundMessageToConversation(const FString& ConversationId, const TArray<uint8>& InputData, const TArray<uint8>& OutputData)
{
	NOT_IMPLEMENTED_RETURN(void())
}

void InworldClientWS::SendAudioSessionStart(const FString& AgentId, FInworldAudioSessionOptions SessionOptions)
{
	NOT_IMPLEMENTED_RETURN(void())
}

void InworldClientWS::SendAudioSessionStartToConversation(const FString& ConversationId, FInworldAudioSessionOptions SessionOptions)
{
	NOT_IMPLEMENTED_RETURN(void())
}

void InworldClientWS::SendAudioSessionStop(const FString& AgentId)
{
	NOT_IMPLEMENTED_RETURN(void())
}

void InworldClientWS::SendAudioSessionStopToConversation(const FString& ConversationId)
{
	NOT_IMPLEMENTED_RETURN(void())
}

void InworldClientWS::SendNarrationEvent(const FString& AgentId, const FString& Content)
{
	NOT_IMPLEMENTED_RETURN(void())
}

void InworldClientWS::SendTrigger(const FString& AgentId, const FString& Name, const TMap<FString, FString>& Params)
{
	NOT_IMPLEMENTED_RETURN(void())
}

void InworldClientWS::SendTriggerToConversation(const FString& ConversationId, const FString& Name, const TMap<FString, FString>& Params)
{
	NOT_IMPLEMENTED_RETURN(void())
}

void InworldClientWS::SendChangeSceneEvent(const FString& SceneName)
{
	NOT_IMPLEMENTED_RETURN(void())
}

void InworldClientWS::CancelResponse(const FString& AgentId, const FString& InteractionId, const TArray<FString>& UtteranceIds)
{
	NOT_IMPLEMENTED_RETURN(void())
}

void InworldClientWS::CreateOrUpdateItems(const TArray<FInworldEntityItem>& Items, const TArray<FString>& AddToEntities)
{
	NOT_IMPLEMENTED_RETURN(void())
}

void InworldClientWS::RemoveItems(const TArray<FString>& ItemIds)
{
	NOT_IMPLEMENTED_RETURN(void())
}

void InworldClientWS::AddItemsInEntities(const TArray<FString>& ItemIds, const TArray<FString>& EntityNames)
{
	NOT_IMPLEMENTED_RETURN(void())
}

void InworldClientWS::RemoveItemsInEntities(const TArray<FString>& ItemIds, const TArray<FString>& EntityNames)
{
	NOT_IMPLEMENTED_RETURN(void())
}

void InworldClientWS::ReplaceItemsInEntities(const TArray<FString>& ItemIds, const TArray<FString>& EntityNames)
{
	NOT_IMPLEMENTED_RETURN(void())
}

EInworldConnectionState InworldClientWS::GetConnectionState() const
{
	NOT_IMPLEMENTED_RETURN(EInworldConnectionState::Idle)
}

void InworldClientWS::GetConnectionError(FString& OutErrorMessage, int32& OutErrorCode, FInworldConnectionErrorDetails& OutErrorDetails) const
{
	NOT_IMPLEMENTED_RETURN(void())
}

static FString GetAuthHeader(const FInworldAuth& AuthOverride)
{
	const UInworldAIClientSettings* InworldAIClientSettings{ GetDefault<UInworldAIClientSettings>() };
	const FInworldAuth& DefaultAuth{ InworldAIClientSettings->Auth };
	const FString& ApiKey{ AuthOverride.ApiKey.IsEmpty() ? DefaultAuth.ApiKey : AuthOverride.ApiKey };
	const FString& ApiSecret{ AuthOverride.ApiSecret.IsEmpty() ? DefaultAuth.ApiSecret : AuthOverride.ApiSecret };

	const FString DateString{ FDateTime::UtcNow().ToString(TEXT("%Y%m%d%H%M%S")) };

	const FString Chars{ FString("1234567890") };
	FString Nonce{ "00000000000" };

	for (int32 i = 0; i < Nonce.Len(); i++)
	{
		Nonce[i] = Chars.GetCharArray()[FMath::RandRange(0, Chars.Len() - 1)];
	}

	const FString Url {"api-engine.inworld.ai"};

	const FString Service{ "ai.inworld.engine.v1.SessionTokens/GenerateSessionToken" };
	const FString Request{ "iw1_request" };

	TAnsiStringBuilder<64> Key;
	Key.Appendf("IW1%s", StringCast<ANSICHAR>(*ApiSecret).Get());
	const Security::FSHA256 DateHash = Security::HmacSha256(StringCast<ANSICHAR>(*DateString).Get(), (const uint8*)*Key, Key.Len());
	const Security::FSHA256 UrlHash = Security::HmacSha256(StringCast<ANSICHAR>(*Url).Get(), DateHash.Hash, sizeof(DateHash.Hash));
	const Security::FSHA256 ServiceHash = Security::HmacSha256(StringCast<ANSICHAR>(*Service).Get(), UrlHash.Hash, sizeof(UrlHash.Hash));
	const Security::FSHA256 NonceHash = Security::HmacSha256(StringCast<ANSICHAR>(*Nonce).Get(), ServiceHash.Hash, sizeof(ServiceHash.Hash));
	const Security::FSHA256 SigningKeyHash = Security::HmacSha256(StringCast<ANSICHAR>(*Request).Get(), NonceHash.Hash, sizeof(NonceHash.Hash));

	FAnsiStringBuilderBase SB;
	SigningKeyHash.ToString(SB);
	FString Signature = SB.ToString();

	return FString("IW1-HMAC-SHA256 ApiKey=") + ApiKey +
		",DateTime=" + DateString +
		",Nonce=" + Nonce +
		",Signature=" + Signature;
}

void InworldClientWS::GenerateToken(FOnTokenGenerated Callback, const FString& WorkspaceOverride, const FInworldAuth& AuthOverride)
{
	const UInworldAIClientSettings* InworldAIClientSettings = GetDefault<UInworldAIClientSettings>();

	TokenRequest = FHttpModule::Get().CreateRequest();
	TokenRequest->SetURL(FString{ "https://" } + InworldAIClientSettings->Environment.ApiUrl + FString{ "/v1/sessionTokens/token:generate" });
	TokenRequest->SetHeader("Authorization", GetAuthHeader(AuthOverride));
	TokenRequest->SetHeader("Content-Type", "application/json");
	TokenRequest->SetVerb("POST");
	const FInworldAuth& DefaultAuth{ InworldAIClientSettings->Auth };
	const FString ApiKey{ AuthOverride.ApiKey.IsEmpty() ? DefaultAuth.ApiKey : AuthOverride.ApiKey };
	const FString& DefaultWorkspace{ InworldAIClientSettings->Workspace };
	const FString Workspace{ WorkspaceOverride.IsEmpty() ? DefaultWorkspace : WorkspaceOverride };
	TokenRequest->SetContentAsString(FString::Printf(TEXT("{\"api_key\":\"%s\", \"resource_id\" : \"workspaces/%s\"}"), *ApiKey, *Workspace));
	TokenRequest->OnProcessRequestComplete().BindLambda(
		[](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
		{
			const int32 ResponseCode = Response->GetResponseCode();
			const FString ResponseString = Response->GetContentAsString();

			const FString ResponseContentString = Response->GetContentAsString();
		}
	);
	TokenRequest->ProcessRequest();
}

#endif
