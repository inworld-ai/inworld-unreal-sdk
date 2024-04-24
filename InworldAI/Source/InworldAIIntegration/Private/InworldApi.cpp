/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldApi.h"
#include "InworldAIIntegrationModule.h"
#include "InworldPackets.h"
#include "InworldCharacter.h"
#include <Engine/Engine.h>
#include <UObject/UObjectGlobals.h>
#include "TimerManager.h"
#include "InworldAudioRepl.h"

static TAutoConsoleVariable<bool> CVarLogAllPackets(
TEXT("Inworld.Debug.LogAllPackets"), false,
TEXT("Enable/Disable logging all packets going from server")
);

UInworldApiSubsystem::UInworldApiSubsystem()
    : Super()
{}

void UInworldApiSubsystem::StartSession(const FString& SceneName, const FString& PlayerName, const FString& ApiKey, const FString& ApiSecret, const FString& AuthUrlOverride, const FString& TargetUrlOverride, const FString& Token, int64 TokenExpirationTime, const FString& SessionId)
{
    if (!ensure(GetWorld()->GetNetMode() < NM_Client))
    {
        UE_LOG(LogInworldAIIntegration, Error, TEXT("UInworldApiSubsystem::StartSession shouldn't be called on client"));
        return;
    }

    if (ApiKey.IsEmpty())
    {
        UE_LOG(LogInworldAIIntegration, Error, TEXT("Can't Start Session, ApiKey is empty"));
        return;
    }
    if (ApiSecret.IsEmpty())
    {
        UE_LOG(LogInworldAIIntegration, Error, TEXT("Can't Start Session, ApiSecret is empty"));
        return;
    }

    FInworldPlayerProfile PlayerProfile;
    PlayerProfile.Name = PlayerName;

    FInworldAuth Auth;
    Auth.ApiKey = ApiKey;
    Auth.ApiSecret = ApiSecret;

    FInworldSessionToken SessionToken;
    SessionToken.Token = Token;
    SessionToken.ExpirationTime = TokenExpirationTime;
    SessionToken.SessionId = SessionId;

    FInworldEnvironment Environment;
    Environment.AuthUrl = AuthUrlOverride;
    Environment.TargetUrl = TargetUrlOverride;

    InworldSession->InworldClient->StartSession(SceneName, PlayerProfile, Auth, FInworldSave(), SessionToken);
}

void UInworldApiSubsystem::StartSession_V2(const FString& SceneName, const FInworldPlayerProfile& PlayerProfile, const FInworldCapabilitySet& Capabilities, const FInworldAuth& Auth, const FInworldSessionToken& SessionToken, const FInworldEnvironment& Environment, FString UniqueUserIdOverride, FInworldSave SavedSessionState)
{
    if (!ensure(GetWorld()->GetNetMode() < NM_Client))
    {
        UE_LOG(LogInworldAIIntegration, Error, TEXT("UInworldApiSubsystem::StartSession shouldn't be called on client"));
        return;
    }

    const bool bValidAuth = !Auth.Base64Signature.IsEmpty() || (!Auth.ApiKey.IsEmpty() && !Auth.ApiSecret.IsEmpty());
    if (!bValidAuth)
    {
        UE_LOG(LogInworldAIIntegration, Error, TEXT("Can't Start Session, either Base64Signature or both ApiKey and ApiSecret need to be not empty"));
        return;
    }
    if (PlayerProfile.ProjectName.IsEmpty())
    {
        UE_LOG(LogInworldAIIntegration, Warning, TEXT("Start Session, please provide unique PlayerProfile.ProjectName for possible troubleshooting"));
    }

    InworldSession->InworldClient->StartSession(SceneName, PlayerProfile, Auth, SavedSessionState, SessionToken, Capabilities);
}

void UInworldApiSubsystem::PauseSession()
{
    InworldSession->InworldClient->PauseSession();
}

void UInworldApiSubsystem::ResumeSession()
{
    InworldSession->InworldClient->PauseSession();
}

void UInworldApiSubsystem::StopSession()
{
    InworldSession->InworldClient->StopSession();
}

void UInworldApiSubsystem::SaveSession(FOnInworldSessionSavedCallback Callback)
{
    InworldSession->InworldClient->SaveSession(Callback);
}

void UInworldApiSubsystem::SetResponseLatencyTrackerDelegate(const FOnInworldPerceivedLatencyCallback& Delegate)
{
    InworldSession->InworldClient->OnPerceivedLatencyDelegate.Add(Delegate);
}

void UInworldApiSubsystem::ClearResponseLatencyTrackerDelegate(const FOnInworldPerceivedLatencyCallback& Delegate)
{
    InworldSession->InworldClient->OnPerceivedLatencyDelegate.Remove(Delegate);
}

void UInworldApiSubsystem::LoadCharacters(const TArray<FString>& Names)
{
    TArray<UInworldCharacter*> Characters;
    for (UInworldCharacter* Character : InworldSession->GetRegisteredCharacters())
    {
        if (Names.Contains(Character->GetBrainName()))
        {
            Characters.Add(Character);
        }
    }
    InworldSession->LoadCharacters(Characters);
}

void UInworldApiSubsystem::UnloadCharacters(const TArray<FString>& Names)
{
    TArray<UInworldCharacter*> Characters;
    for (UInworldCharacter* Character : InworldSession->GetRegisteredCharacters())
    {
        if (Names.Contains(Character->GetBrainName()))
        {
            Characters.Add(Character);
        }
    }
	InworldSession->UnloadCharacters(Characters);
}

void UInworldApiSubsystem::LoadSavedState(const FInworldSave& SavedState)
{
    InworldSession->InworldClient->LoadSavedState(SavedState);
}

void UInworldApiSubsystem::LoadCapabilities(const FInworldCapabilitySet& Capabilities)
{
    InworldSession->InworldClient->LoadCapabilities(Capabilities);
}

void UInworldApiSubsystem::LoadPlayerProfile(const FInworldPlayerProfile& PlayerProfile)
{
    InworldSession->InworldClient->LoadPlayerProfile(PlayerProfile);
}

void UInworldApiSubsystem::SendTextMessage(const FString& AgentId, const FString& Text)
{
    SendTextMessageMultiAgent({ AgentId }, Text);
}

void UInworldApiSubsystem::SendTextMessageMultiAgent(const TArray<FString>& AgentIds, const FString& Text)
{
    if (!ensureMsgf(AgentIds.Num() != 0, TEXT("AgentIds must be valid!")))
    {
        return;
    }

    InworldSession->InworldClient->SendTextMessage(AgentIds, Text).Packet;
}

void UInworldApiSubsystem::SendTrigger(const FString& AgentId, const FString& Name, const TMap<FString, FString>& Params)
{
    SendTriggerMultiAgent({ AgentId }, Name, Params);
}

void UInworldApiSubsystem::SendTriggerMultiAgent(const TArray<FString>& AgentIds, const FString& Name, const TMap<FString, FString>& Params)
{
    if (!ensureMsgf(AgentIds.Num() != 0, TEXT("AgentId must be valid!")))
    {
        return;
    }

    InworldSession->InworldClient->BroadcastTrigger(AgentIds, Name, Params);
}

void UInworldApiSubsystem::SendNarrationEvent(const FString& AgentId, const FString& Content)
{
    if (!ensureMsgf(!AgentId.IsEmpty(), TEXT("AgentId must be valid!")))
    {
        return;
    }

    InworldSession->InworldClient->SendNarrationEvent(AgentId, Content);
}

void UInworldApiSubsystem::SendAudioMessage(const TArray<FString>& AgentIds, const TArray<uint8>& InputData, const TArray<uint8>& OutputData)
{
    if (!ensureMsgf(AgentIds.Num() != 0, TEXT("AgentIds must be valid!")))
    {
        return;
    }

    InworldSession->InworldClient->BroadcastSoundMessage(AgentIds, InputData, OutputData);
}

bool UInworldApiSubsystem::StartAudioSession(const FString& AgentId, const AActor* Owner)
{
    return StartAudioSessionMultiAgent({ AgentId }, Owner);
}

bool UInworldApiSubsystem::StartAudioSessionMultiAgent(const TArray<FString>& AgentIds, const AActor* Owner)
{
    if (AudioSessionOwner)
    {
        return false;
    }

    if (!ensureMsgf(AgentIds.Num() != 0, TEXT("AgentIds must be valid!")))
    {
        return false;
    }

    AudioSessionOwner = Owner;

    InworldSession->InworldClient->BroadcastAudioSessionStart(AgentIds);
    return true;
}

void UInworldApiSubsystem::StopAudioSession(const FString& AgentId)
{
    StopAudioSessionMultiAgent({ AgentId });
}

void UInworldApiSubsystem::StopAudioSessionMultiAgent(const TArray<FString>& AgentIds)
{
    if (!ensureMsgf(AgentIds.Num() != 0, TEXT("AgentIds must be valid!")))
    {
        return;
    }

    AudioSessionOwner = nullptr;

    InworldSession->InworldClient->BroadcastAudioSessionStop(AgentIds);
}

void UInworldApiSubsystem::ChangeScene(const FString& SceneId)
{
    InworldSession->InworldClient->SendChangeSceneEvent(SceneId);
}

void UInworldApiSubsystem::GetConnectionError(FString& Message, int32& Code)
{
    InworldSession->InworldClient->GetConnectionError(Message, Code);
}

void UInworldApiSubsystem::CancelResponse(const FString& AgentId, const FString& InteractionId, const TArray<FString>& UtteranceIds)
{
	if (!ensureMsgf(!AgentId.IsEmpty(), TEXT("AgentId must be valid!")))
	{
		return;
	}

    InworldSession->InworldClient->CancelResponse(AgentId, InteractionId, UtteranceIds);
}

void UInworldApiSubsystem::StartAudioReplication()
{
	if (!AudioRepl && GetWorld()->GetNetMode() != NM_Standalone)
	{
		AudioRepl = NewObject<UInworldAudioRepl>(GetWorld(), TEXT("InworldAudioRepl"));
	}
}

bool UInworldApiSubsystem::DoesSupportWorldType(EWorldType::Type WorldType) const
{
    return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UInworldApiSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    if (GetWorld()->GetNetMode() == NM_Client)
    {
        return;
    }
    InworldSession = NewObject<UInworldSession>(this, TEXT("InworldSession"));
    InworldSession->OnCharactersInitialized().AddLambda(
        [this](bool bCharactersInitialized) -> void
        {
            OnCharactersInitialized.Broadcast(bCharactersInitialized);
        }
    );
}

void UInworldApiSubsystem::Deinitialize()
{
    Super::Deinitialize();
    InworldSession = nullptr;
    AudioSessionOwner = nullptr;
}

#if ENGINE_MAJOR_VERSION > 4
void UInworldApiSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);

    if (GetWorld()->GetNetMode() != NM_Standalone)
    {
        StartAudioReplication();
    }
}
#endif

/*
void UInworldApiSubsystem::DispatchPacket(TSharedPtr<FInworldPacket> InworldPacket)
{
	auto* SourceComponentPtr = CharacterComponentByAgentId.Find(InworldPacket->Routing.Source.Name);
	if (SourceComponentPtr)
	{
		(*SourceComponentPtr)->HandlePacket(InworldPacket);
	}

    if (InworldPacket->Routing.Source.Type == EInworldActorType::PLAYER)
    {
        auto ProcessTarget = [this, InworldPacket](const FInworldActor& TargetActor)
            {
                auto* TargetComponentPtr = CharacterComponentByAgentId.Find(TargetActor.Name);
                if (TargetComponentPtr)
                {
                    (*TargetComponentPtr)->HandlePacket(InworldPacket);
                }
            };

        ProcessTarget(InworldPacket->Routing.Target);

        for (const auto& Target : InworldPacket->Routing.Targets)
        {
            if (Target.Name != InworldPacket->Routing.Target.Name)
            {
                ProcessTarget(Target);
            }
        }
    }

    if (ensure(InworldPacket))
    {
        InworldPacket->Accept(*this);
    }
}

void UInworldApiSubsystem::Visit(const FInworldChangeSceneEvent& Event)
{
    UnpossessAgents();
    PossessAgents(Event.AgentInfos);
}

void UInworldApiSubsystem::Visit(const FInworldLoadCharactersEvent& Event)
{
    PossessAgents(Event.AgentInfos);
}
*/

void UInworldApiSubsystem::ReplicateAudioEventFromServer(FInworldAudioDataEvent& Packet)
{
    if (AudioRepl)
    {
        AudioRepl->ReplicateAudioEvent(Packet);
    }
}

void UInworldApiSubsystem::HandleAudioEventOnClient(TSharedPtr<FInworldAudioDataEvent> Packet)
{
    //TODO: FIX
    //DispatchPacket(Packet);
}
