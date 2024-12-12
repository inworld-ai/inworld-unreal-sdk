/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldClient.h"
#include "InworldAIClientModule.h"
#include "InworldMacros.h"

#if defined(INWORLD_WITH_NDK) && defined(INWORLD_WITH_WS)
#error Only one client type can be defined.
#endif

#if defined(INWORLD_WITH_NDK)
#include "NDK/InworldClientNDK.h"
#elif defined(INWORLD_WITH_WS)
#include "WS/InworldClientWS.h"
#else
#error A client type must be defined.
#endif

#include "CoreMinimal.h"

#define EMPTY_ARG_RETURN(Arg, Return) INWORLD_WARN_AND_RETURN_EMPTY(LogInworldAIClient, UInworldClient, Arg, Return)
#define NO_CLIENT_RETURN(Return) EMPTY_ARG_RETURN(Client, Return)

UInworldClient::UInworldClient()
{
#if defined(INWORLD_WITH_NDK)
	Client = MakeUnique<InworldClientNDK>();
#elif defined(INWORLD_WITH_WS)
	Client = MakeUnique<InworldClientWS>();
#else
#error A client type must be defined.
#endif

	if (Client.IsValid())
	{
		Client->OnConnectionStateChanged().AddLambda([this](EInworldConnectionState ConnectionState)
			{
				UE_LOG(LogInworldAIClient, Warning, TEXT("CONNECTION STATE: %d"), (int32)ConnectionState);
				if (bIsBeingDestroyed)
				{
					return;
				}

				AsyncTask(ENamedThreads::GameThread, [this, ConnectionState]()
					{
						if (bIsBeingDestroyed)
						{
							return;
						}
						OnConnectionStateChangedDelegateNative.Broadcast(static_cast<EInworldConnectionState>(ConnectionState));
						OnConnectionStateChangedDelegate.Broadcast(static_cast<EInworldConnectionState>(ConnectionState));
					}
				);
			}
		);

		Client->OnPacketReceived().AddLambda([this](const FInworldWrappedPacket& WrappedPacket)
			{
				if (bIsBeingDestroyed)
				{
					return;
				}
				auto Packet = WrappedPacket.Packet;
				AsyncTask(ENamedThreads::GameThread, [this, Packet]()
					{
						if (bIsBeingDestroyed)
						{
							return;
						}
						OnPacketReceivedDelegateNative.Broadcast(Packet);
						OnPacketReceivedDelegate.Broadcast(Packet);
					});
			}
		);

		Client->OnPerceivedLatency().AddLambda([this](FString InteractionId, int32 LatencyMs) {
			OnPerceivedLatencyDelegateNative.Broadcast(InteractionId, LatencyMs);
			OnPerceivedLatencyDelegate.Broadcast(InteractionId, LatencyMs);
		});
	}
}

UInworldClient::~UInworldClient()
{
	bIsBeingDestroyed = true;
	Client.Reset();
}

void UInworldClient::StartSessionFromScene(const FInworldScene& Scene, const FInworldPlayerProfile& PlayerProfile, const FInworldCapabilitySet& CapabilitySet, const TMap<FString, FString>& Metadata, const FString& WorkspaceOverride, const FInworldAuth& AuthOverride)
{
	NO_CLIENT_RETURN(void())
	Client->StartSessionFromScene(Scene, PlayerProfile, CapabilitySet, Metadata, WorkspaceOverride, AuthOverride);
}

void UInworldClient::StartSessionFromSave(const FInworldSave& Save, const FInworldPlayerProfile& PlayerProfile, const FInworldCapabilitySet& CapabilitySet, const TMap<FString, FString>& Metadata, const FString& WorkspaceOverride, const FInworldAuth& AuthOverride)
{
	NO_CLIENT_RETURN(void())
	Client->StartSessionFromSave(Save, PlayerProfile, CapabilitySet, Metadata, WorkspaceOverride, AuthOverride);
}

void UInworldClient::StartSessionFromToken(const FInworldToken& Token, const FInworldPlayerProfile& PlayerProfile, const FInworldCapabilitySet& CapabilitySet, const TMap<FString, FString>& Metadata, const FString& WorkspaceOverride, const FInworldAuth& AuthOverride)
{
	NO_CLIENT_RETURN(void())
	Client->StartSessionFromToken(Token, PlayerProfile, CapabilitySet, Metadata, WorkspaceOverride, AuthOverride);
}

void UInworldClient::StopSession()
{
	NO_CLIENT_RETURN(void())
	OnPreStopDelegateNative.Broadcast();
	OnPreStopDelegate.Broadcast();
	Client->StopSession();
}

void UInworldClient::PauseSession()
{
	NO_CLIENT_RETURN(void())
	OnPrePauseDelegateNative.Broadcast();
	OnPrePauseDelegate.Broadcast();
	Client->PauseSession();
}

void UInworldClient::ResumeSession()
{
	NO_CLIENT_RETURN(void())
	Client->ResumeSession();
}

FInworldToken UInworldClient::GetSessionToken() const
{
	NO_CLIENT_RETURN({})
	return Client->GetSessionToken();
}

void UInworldClient::LoadPlayerProfile(const FInworldPlayerProfile& PlayerProfile)
{
	NO_CLIENT_RETURN(void())
	Client->LoadPlayerProfile(PlayerProfile);
}

FInworldCapabilitySet UInworldClient::GetCapabilities() const
{
	NO_CLIENT_RETURN({})
	return Client->GetCapabilities();
}

void UInworldClient::LoadCapabilities(const FInworldCapabilitySet& CapabilitySet)
{
	NO_CLIENT_RETURN(void())
	Client->LoadCapabilities(CapabilitySet);
}

void UInworldClient::SaveSession(FOnInworldSessionSavedCallback Callback)
{
	NO_CLIENT_RETURN(void())
	Client->SaveSession(Callback);
}

void UInworldClient::SendInteractionFeedback(const FString& InteractionId, bool bIsLike, const FString& Message)
{
	NO_CLIENT_RETURN(void())
	EMPTY_ARG_RETURN(InteractionId, void())
	Client->SendInteractionFeedback(InteractionId, bIsLike, Message);
}

void UInworldClient::LoadCharacter(const FString& Id)
{
	NO_CLIENT_RETURN(void())
	EMPTY_ARG_RETURN(Id, void())
	Client->LoadCharacter(Id);
}

void UInworldClient::LoadCharacters(const TArray<FString>& Ids)
{
	NO_CLIENT_RETURN(void())
	EMPTY_ARG_RETURN(Ids, void())
	Client->LoadCharacters(Ids);
}

void UInworldClient::UnloadCharacter(const FString& Id)
{
	NO_CLIENT_RETURN(void())
	EMPTY_ARG_RETURN(Id, void())
	Client->UnloadCharacter(Id);
}

void UInworldClient::UnloadCharacters(const TArray<FString>& Ids)
{
	NO_CLIENT_RETURN(void())
	EMPTY_ARG_RETURN(Ids, void())
	Client->UnloadCharacters(Ids);
}

FString UInworldClient::UpdateConversation(const FString& ConversationId, const TArray<FString>& AgentIds, bool bIncludePlayer)
{
	NO_CLIENT_RETURN({})
	EMPTY_ARG_RETURN(AgentIds, {})
	return Client->UpdateConversation(ConversationId, AgentIds, bIncludePlayer);
}

FInworldWrappedPacket UInworldClient::SendTextMessage(const FString& AgentId, const FString& Text)
{
	NO_CLIENT_RETURN({})
	EMPTY_ARG_RETURN(AgentId, {})
	EMPTY_ARG_RETURN(Text, {})
	return Client->SendTextMessage(AgentId, Text);
}

FInworldWrappedPacket UInworldClient::SendTextMessageToConversation(const FString& ConversationId, const FString& Text)
{
	NO_CLIENT_RETURN({})
	EMPTY_ARG_RETURN(ConversationId, {})
	EMPTY_ARG_RETURN(Text, {})
	return Client->SendTextMessageToConversation(ConversationId, Text);
}

void UInworldClient::InitSpeechProcessor(EInworldPlayerSpeechMode Mode, const FInworldPlayerSpeechOptions& SpeechOptions)
{
	NO_CLIENT_RETURN(void())
	Client->InitSpeechProcessor(Mode, SpeechOptions);
}

void UInworldClient::DestroySpeechProcessor()
{
	NO_CLIENT_RETURN(void())
	Client->DestroySpeechProcessor();
}

void UInworldClient::SendSoundMessage(const FString& AgentId, const TArray<uint8>& InputData, const TArray<uint8>& OutputData)
{
	NO_CLIENT_RETURN(void())
	EMPTY_ARG_RETURN(AgentId, void())
	EMPTY_ARG_RETURN(InputData, void())
	Client->SendSoundMessage(AgentId, InputData, OutputData);
}

void UInworldClient::SendSoundMessageToConversation(const FString& ConversationId, const TArray<uint8>& InputData, const TArray<uint8>& OutputData)
{
	NO_CLIENT_RETURN(void())
	EMPTY_ARG_RETURN(ConversationId, void())
	EMPTY_ARG_RETURN(InputData, void())
	Client->SendSoundMessageToConversation(ConversationId, InputData, OutputData);
}

void UInworldClient::SendAudioSessionStart(const FString& AgentId, FInworldAudioSessionOptions SessionOptions)
{
	NO_CLIENT_RETURN(void())
	EMPTY_ARG_RETURN(AgentId, void())
	Client->SendAudioSessionStart(AgentId, SessionOptions);
}

void UInworldClient::SendAudioSessionStartToConversation(const FString& ConversationId, FInworldAudioSessionOptions SessionOptions)
{
	NO_CLIENT_RETURN(void())
	EMPTY_ARG_RETURN(ConversationId, void())
	Client->SendAudioSessionStartToConversation(ConversationId, SessionOptions);
}

void UInworldClient::SendAudioSessionStop(const FString& AgentId)
{
	NO_CLIENT_RETURN(void())
	EMPTY_ARG_RETURN(AgentId, void())
	Client->SendAudioSessionStop(AgentId);
}

void UInworldClient::SendAudioSessionStopToConversation(const FString& ConversationId)
{
	NO_CLIENT_RETURN(void())
	EMPTY_ARG_RETURN(ConversationId, void())
	Client->SendAudioSessionStopToConversation(ConversationId);
}

void UInworldClient::SendNarrationEvent(const FString& AgentId, const FString& Content)
{
	NO_CLIENT_RETURN(void())
	EMPTY_ARG_RETURN(AgentId, void())
	EMPTY_ARG_RETURN(Content, void())
	Client->SendNarrationEvent(AgentId, Content);
}

void UInworldClient::SendTrigger(const FString& AgentId, const FString& Name, const TMap<FString, FString>& Params)
{
	NO_CLIENT_RETURN(void())
	EMPTY_ARG_RETURN(AgentId, void())
	EMPTY_ARG_RETURN(Name, void())
	Client->SendTrigger(AgentId, Name, Params);
}

void UInworldClient::SendTriggerToConversation(const FString& ConversationId, const FString& Name, const TMap<FString, FString>& Params)
{
	NO_CLIENT_RETURN(void())
	EMPTY_ARG_RETURN(ConversationId, void())
	EMPTY_ARG_RETURN(Name, void())
	Client->SendTriggerToConversation(ConversationId, Name, Params);
}

void UInworldClient::SendChangeSceneEvent(const FString& SceneName)
{
	NO_CLIENT_RETURN(void())
	EMPTY_ARG_RETURN(SceneName, void())
	Client->SendChangeSceneEvent(SceneName);
}

void UInworldClient::CancelResponse(const FString& AgentId, const FString& InteractionId, const TArray<FString>& UtteranceIds)
{
	NO_CLIENT_RETURN(void())
	EMPTY_ARG_RETURN(AgentId, void())
	EMPTY_ARG_RETURN(InteractionId, void())
	EMPTY_ARG_RETURN(UtteranceIds, void())
	Client->CancelResponse(AgentId, InteractionId, UtteranceIds);
}

void UInworldClient::CreateOrUpdateItems(const TArray<FInworldEntityItem>& Items, const TArray<FString>& AddToEntities)
{
	NO_CLIENT_RETURN(void())
	EMPTY_ARG_RETURN(Items, void())
	EMPTY_ARG_RETURN(AddToEntities, void())
	Client->CreateOrUpdateItems(Items, AddToEntities);
}

void UInworldClient::RemoveItems(const TArray<FString>& ItemIds)
{
	NO_CLIENT_RETURN(void())
	EMPTY_ARG_RETURN(ItemIds, void())
	Client->RemoveItems(ItemIds);
}

void UInworldClient::AddItemsInEntities(const TArray<FString>& ItemIds, const TArray<FString>& EntityNames)
{
	NO_CLIENT_RETURN(void())
	EMPTY_ARG_RETURN(ItemIds, void())
	EMPTY_ARG_RETURN(EntityNames, void())
	Client->AddItemsInEntities(ItemIds, EntityNames);
}

void UInworldClient::RemoveItemsInEntities(const TArray<FString>& ItemIds, const TArray<FString>& EntityNames)
{
	NO_CLIENT_RETURN(void())
	EMPTY_ARG_RETURN(ItemIds, void())
	EMPTY_ARG_RETURN(EntityNames, void())
	Client->RemoveItemsInEntities(ItemIds, EntityNames);
}

void UInworldClient::ReplaceItemsInEntities(const TArray<FString>& ItemIds, const TArray<FString>& EntityNames)
{
	NO_CLIENT_RETURN(void())
	EMPTY_ARG_RETURN(ItemIds, void())
	EMPTY_ARG_RETURN(EntityNames, void())
	Client->ReplaceItemsInEntities(ItemIds, EntityNames);
}

EInworldConnectionState UInworldClient::GetConnectionState() const
{
	NO_CLIENT_RETURN(EInworldConnectionState::Idle)
	return Client->GetConnectionState();
}

void UInworldClient::GetConnectionError(FString& OutErrorMessage, int32& OutErrorCode, FInworldConnectionErrorDetails& OutErrorDetails) const
{
	NO_CLIENT_RETURN(void())
	Client->GetConnectionError(OutErrorMessage, OutErrorCode, OutErrorDetails);
}

#undef EMPTY_ARG_RETURN
#undef NO_CLIENT_RETURN
