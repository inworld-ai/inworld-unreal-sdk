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
#include "InworldSessionComponent.h"
#include <Engine/Engine.h>
#include <UObject/UObjectGlobals.h>
#include "TimerManager.h"
#include "InworldAudioRepl.h"
#include "UObject/UObjectIterator.h"
#include "GameFramework/GameStateBase.h"

static TAutoConsoleVariable<bool> CVarLogAllPackets(
TEXT("Inworld.Debug.LogAllPackets"), false,
TEXT("Enable/Disable logging all packets going from server")
);

UInworldApiSubsystem::UInworldApiSubsystem()
    : Super()
{}

void UInworldApiSubsystem::SetInworldSession(UInworldSession* Session)
{
    if (InworldSession != Session)
    {
        InworldSession = Session;
        InworldSession->OnLoaded().AddLambda(
            [this](bool bLoaded) -> void
            {
                OnCharactersInitialized.Broadcast(bLoaded);
            }
        );
        OnCharactersInitialized.Broadcast(InworldSession->IsLoaded());
        InworldSession->OnConnectionStateChanged().AddLambda(
            [this](EInworldConnectionState ConnectionState) -> void
            {
                OnConnectionStateChanged.Broadcast(ConnectionState);
            }
        );
    }
}

void UInworldApiSubsystem::StartSession(const FString& SceneName, const FString& PlayerName, const FString& ApiKey, const FString& ApiSecret, const FString& AuthUrlOverride, const FString& TargetUrlOverride, const FString& Token, int64 TokenExpirationTime, const FString& SessionId)
{
    if (!ensureMsgf(InworldSession && InworldSession->InworldClient, TEXT("Inworld Session and Inworld Client must be valid!")))
    {
        return;
    }

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

    InworldSession->InworldClient->StartSession(SceneName, PlayerProfile, Auth, FInworldSave(), SessionToken, {});
}

void UInworldApiSubsystem::StartSession_V2(const FString& SceneName, const FInworldPlayerProfile& PlayerProfile, const FInworldCapabilitySet& Capabilities, const FInworldAuth& Auth, const FInworldSessionToken& SessionToken, const FInworldEnvironment& Environment, FString UniqueUserIdOverride, FInworldSave SavedSessionState)
{
    if (!ensureMsgf(InworldSession && InworldSession->InworldClient, TEXT("Inworld Session and Inworld Client must be valid!")))
    {
        return;
    }

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
    if (!ensureMsgf(InworldSession && InworldSession->InworldClient, TEXT("Inworld Session and Inworld Client must be valid!")))
    {
        return;
    }

    InworldSession->InworldClient->PauseSession();
}

void UInworldApiSubsystem::ResumeSession()
{
    if (!ensureMsgf(InworldSession && InworldSession->InworldClient, TEXT("Inworld Session and Inworld Client must be valid!")))
    {
        return;
    }

    InworldSession->InworldClient->PauseSession();
}

void UInworldApiSubsystem::StopSession()
{
    if (!ensureMsgf(InworldSession && InworldSession->InworldClient, TEXT("Inworld Session and Inworld Client must be valid!")))
    {
        return;
    }

    InworldSession->InworldClient->StopSession();
}

void UInworldApiSubsystem::SaveSession(FOnInworldSessionSavedCallback Callback)
{
    if (!ensureMsgf(InworldSession && InworldSession->InworldClient, TEXT("Inworld Session and Inworld Client must be valid!")))
    {
        return;
    }

    InworldSession->InworldClient->SaveSession(Callback);
}

void UInworldApiSubsystem::SetResponseLatencyTrackerDelegate(const FOnInworldPerceivedLatencyCallback& Delegate)
{
    if (!ensureMsgf(InworldSession && InworldSession->InworldClient, TEXT("Inworld Session and Inworld Client must be valid!")))
    {
        return;
    }

    InworldSession->InworldClient->OnPerceivedLatencyDelegate.Add(Delegate);
}

void UInworldApiSubsystem::ClearResponseLatencyTrackerDelegate(const FOnInworldPerceivedLatencyCallback& Delegate)
{
    if (!ensureMsgf(InworldSession && InworldSession->InworldClient, TEXT("Inworld Session and Inworld Client must be valid!")))
    {
        return;
    }

    InworldSession->InworldClient->OnPerceivedLatencyDelegate.Remove(Delegate);
}

void UInworldApiSubsystem::LoadCharacters(const TArray<FString>& Names)
{
    if (!ensureMsgf(InworldSession && InworldSession->InworldClient, TEXT("Inworld Session and Inworld Client must be valid!")))
    {
        return;
    }

    TArray<UInworldCharacter*> Characters;
    for (UInworldCharacter* Character : InworldSession->GetRegisteredCharacters())
    {
        if (Names.Contains(Character->GetAgentInfo().BrainName))
        {
            Characters.Add(Character);
        }
    }
    InworldSession->LoadCharacters(Characters);
}

void UInworldApiSubsystem::UnloadCharacters(const TArray<FString>& Names)
{
    if (!ensureMsgf(InworldSession && InworldSession->InworldClient, TEXT("Inworld Session and Inworld Client must be valid!")))
    {
        return;
    }

    TArray<UInworldCharacter*> Characters;
    for (UInworldCharacter* Character : InworldSession->GetRegisteredCharacters())
    {
        if (Names.Contains(Character->GetAgentInfo().BrainName))
        {
            Characters.Add(Character);
        }
    }
	InworldSession->UnloadCharacters(Characters);
}

void UInworldApiSubsystem::LoadSavedState(const FInworldSave& SavedState)
{
    if (!ensureMsgf(InworldSession && InworldSession->InworldClient, TEXT("Inworld Session and Inworld Client must be valid!")))
    {
        return;
    }

    InworldSession->InworldClient->LoadSavedState(SavedState);
}

void UInworldApiSubsystem::LoadCapabilities(const FInworldCapabilitySet& Capabilities)
{
    if (!ensureMsgf(InworldSession && InworldSession->InworldClient, TEXT("Inworld Session and Inworld Client must be valid!")))
    {
        return;
    }

    InworldSession->InworldClient->LoadCapabilities(Capabilities);
}

void UInworldApiSubsystem::LoadPlayerProfile(const FInworldPlayerProfile& PlayerProfile)
{
    if (!ensureMsgf(InworldSession && InworldSession->InworldClient, TEXT("Inworld Session and Inworld Client must be valid!")))
    {
        return;
    }

    InworldSession->InworldClient->LoadPlayerProfile(PlayerProfile);
}

void UInworldApiSubsystem::SendTextMessage(const FString& AgentId, const FString& Text)
{
    InworldSession->InworldClient->SendTextMessage(AgentId, Text);
}

void UInworldApiSubsystem::SendTrigger(const FString& AgentId, const FString& Name, const TMap<FString, FString>& Params)
{
    InworldSession->InworldClient->SendTrigger(AgentId, Name, Params);
}

void UInworldApiSubsystem::SendNarrationEvent(const FString& AgentId, const FString& Content)
{
    if (!ensureMsgf(InworldSession && InworldSession->InworldClient, TEXT("Inworld Session and Inworld Client must be valid!")))
    {
        return;
    }

    if (!ensureMsgf(!AgentId.IsEmpty(), TEXT("AgentId must be valid!")))
    {
        return;
    }

    InworldSession->InworldClient->SendNarrationEvent(AgentId, Content);
}

void UInworldApiSubsystem::SendAudioMessage(const FString& AgentId, const TArray<uint8>& InputData, const TArray<uint8>& OutputData)
{
    if (!ensureMsgf(InworldSession && InworldSession->InworldClient, TEXT("Inworld Session and Inworld Client must be valid!")))
    {
        return;
    }

    InworldSession->InworldClient->SendSoundMessage(AgentId, InputData, OutputData);
}

void UInworldApiSubsystem::StartAudioSession(const FString& AgentId)
{
    InworldSession->InworldClient->SendAudioSessionStart(AgentId);
}

void UInworldApiSubsystem::StopAudioSession(const FString& AgentId)
{
    InworldSession->InworldClient->SendAudioSessionStop(AgentId);
}

void UInworldApiSubsystem::ChangeScene(const FString& SceneId)
{
    if (!ensureMsgf(InworldSession && InworldSession->InworldClient, TEXT("Inworld Session and Inworld Client must be valid!")))
    {
        return;
    }

    InworldSession->InworldClient->SendChangeSceneEvent(SceneId);
}

EInworldConnectionState UInworldApiSubsystem::GetConnectionState() const
{
    if (!ensureMsgf(InworldSession && InworldSession->InworldClient, TEXT("Inworld Session and Inworld Client must be valid!")))
    {
        return EInworldConnectionState::Idle;
    }

    return InworldSession->GetConnectionState();
}

void UInworldApiSubsystem::GetConnectionError(FString& Message, int32& Code)
{
    if (!ensureMsgf(InworldSession && InworldSession->InworldClient, TEXT("Inworld Session and Inworld Client must be valid!")))
    {
        return;
    }

    InworldSession->InworldClient->GetConnectionError(Message, Code);
}

void UInworldApiSubsystem::CancelResponse(const FString& AgentId, const FString& InteractionId, const TArray<FString>& UtteranceIds)
{
    if (!ensureMsgf(InworldSession && InworldSession->InworldClient, TEXT("Inworld Session and Inworld Client must be valid!")))
    {
        return;
    }

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
}

void UInworldApiSubsystem::Deinitialize()
{
    Super::Deinitialize();
    InworldSession = nullptr;
}

#if ENGINE_MAJOR_VERSION > 4
void UInworldApiSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);

    UWorld* World = GetWorld();
    if (InworldSession == nullptr && World && World->GetNetMode() != NM_Client)
    {
        // Backward compatibility for calls directly to InworldApi without some sort of session actor
        UInworldSessionComponent* SessionComponent = Cast<UInworldSessionComponent>(World->GetGameState()->AddComponentByClass(UInworldSessionComponent::StaticClass(), false, FTransform::Identity, false));
        SetInworldSession(IInworldSessionOwnerInterface::Execute_GetInworldSession(SessionComponent));
    }

    if (World && World->GetNetMode() != NM_Standalone)
    {
        StartAudioReplication();
    }
}
#endif

void UInworldApiSubsystem::ReplicateAudioEventFromServer(FInworldAudioDataEvent& Packet)
{
    if (AudioRepl)
    {
        AudioRepl->ReplicateAudioEvent(Packet);
    }
}

void UInworldApiSubsystem::HandleAudioEventOnClient(TSharedPtr<FInworldAudioDataEvent> Packet)
{
    if (!ensureMsgf(InworldSession, TEXT("Inworld Session must be valid!")))
    {
        return;
    }
    InworldSession->HandlePacket(FInworldWrappedPacket(Packet));
}
