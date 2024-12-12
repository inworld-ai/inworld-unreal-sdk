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

#define NOT_IMPLEMENTED_RETURN(Return) UE_LOG(LogInworldAIClient, Warning, TEXT("InworldClientWS::%s skipped: due to lack of WebSocket support."), *FString(__func__)); return Return;

InworldClientWS::InworldClientWS()
{

}

InworldClientWS::~InworldClientWS()
{

}

void InworldClientWS::StartSessionFromScene(const FInworldScene& Scene, const FInworldPlayerProfile& PlayerProfile, const FInworldCapabilitySet& CapabilitySet, const TMap<FString, FString>& Metadata, const FString& WorkspaceOverride, const FInworldAuth& AuthOverride)
{
	NOT_IMPLEMENTED_RETURN(void())
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

#endif
