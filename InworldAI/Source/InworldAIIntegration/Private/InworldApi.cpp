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
#include "InworldPlayer.h"
#include "InworldSessionComponent.h"
#include "InworldMacros.h"
#include <Engine/Engine.h>
#include <UObject/UObjectGlobals.h>
#include "TimerManager.h"
#include "InworldAudioRepl.h"
#include "UObject/UObjectIterator.h"
#include "GameFramework/GameStateBase.h"
#include "Runtime/Launch/Resources/Version.h"

static TAutoConsoleVariable<bool> CVarLogAllPackets(
TEXT("Inworld.Debug.LogAllPackets"), false,
TEXT("Enable/Disable logging all packets going from server")
);

#define EMPTY_ARG_RETURN(Arg, Return) INWORLD_WARN_AND_RETURN_EMPTY(LogInworldAIIntegration, UInworldApiSubsystem, Arg, Return)
#define NO_SESSION_RETURN(Return) EMPTY_ARG_RETURN(InworldSession, Return)
#define NO_CLIENT_RETURN(Return) NO_SESSION_RETURN(Return) EMPTY_ARG_RETURN(InworldSession->GetClient(), Return)

UInworldApiSubsystem::UInworldApiSubsystem()
    : Super()
    , AudioRepl(nullptr)
    , InworldSession(nullptr)
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

UInworldSession* UInworldApiSubsystem::GetInworldSession()
{
    if (InworldSession == nullptr)
    {
        // Lazily instantiate the InworldSession if needed for backwards compatability
        UWorld* World = GetWorld();
        if (World && World->GetNetMode() != NM_Client)
        {
            // Backward compatibility for calls directly to InworldApi without some sort of session actor
            UInworldSessionComponent* SessionComponent = Cast<UInworldSessionComponent>(World->GetGameState()->AddComponentByClass(UInworldSessionComponent::StaticClass(), false, FTransform::Identity, false));
            SetInworldSession(IInworldSessionOwnerInterface::Execute_GetInworldSession(SessionComponent));
        }
    }
    return InworldSession;
}

void UInworldApiSubsystem::StartSession(const FString& SceneName, const FString& PlayerName, const FString& ApiKey, const FString& ApiSecret, const FString& Token, int64 TokenExpirationTime, const FString& SessionId)
{
    NO_CLIENT_RETURN(void())

    FInworldPlayerProfile PlayerProfile;
    PlayerProfile.Name = PlayerName;

    FInworldAuth Auth;
    Auth.ApiKey = ApiKey;
    Auth.ApiSecret = ApiSecret;

    FInworldToken SessionToken;
    SessionToken.Token = Token;
    SessionToken.ExpirationTime = TokenExpirationTime;
    SessionToken.SessionId = SessionId;

    StartSession_V2(SceneName, PlayerProfile, {}, Auth, SessionToken, {}, {});
}

void UInworldApiSubsystem::StartSession_V2(const FString& SceneName, const FInworldPlayerProfile& PlayerProfile, const FInworldCapabilitySet& Capabilities, const FInworldAuth& Auth, const FInworldToken& SessionToken, FString UniqueUserIdOverride, FInworldSave SavedSessionState)
{
    NO_CLIENT_RETURN(void())

    if (PlayerProfile.ProjectName.IsEmpty())
    {
        UE_LOG(LogInworldAIIntegration, Warning, TEXT("Start Session, please provide unique PlayerProfile.ProjectName for possible troubleshooting"));
    }

    if (!SavedSessionState.State.IsEmpty())
    {
        InworldSession->GetClient()->StartSessionFromSave(SavedSessionState, PlayerProfile, Capabilities, {}, {}, Auth);
    }
    else if (!SessionToken.Token.IsEmpty())
    {
        InworldSession->GetClient()->StartSessionFromToken(SessionToken, PlayerProfile, Capabilities, {}, {}, Auth);
    }
    else
    {
        FInworldScene Scene;
        Scene.Name = SceneName;
        InworldSession->GetClient()->StartSessionFromScene(Scene, PlayerProfile, Capabilities, {}, {}, Auth);
    }
}

void UInworldApiSubsystem::PauseSession()
{
    NO_CLIENT_RETURN(void())

    InworldSession->PauseSession();
}

void UInworldApiSubsystem::ResumeSession()
{
    NO_CLIENT_RETURN(void())

    InworldSession->ResumeSession();
}

void UInworldApiSubsystem::StopSession()
{
    NO_CLIENT_RETURN(void())

    InworldSession->StopSession();
}

void UInworldApiSubsystem::SaveSession(FOnInworldSessionSavedCallback Callback)
{
    NO_CLIENT_RETURN(void())

    InworldSession->SaveSession(Callback);
}

void UInworldApiSubsystem::SetResponseLatencyTrackerDelegate(const FOnInworldPerceivedLatencyCallback& Delegate)
{
    NO_CLIENT_RETURN(void())

    InworldSession->OnPerceivedLatencyDelegate.Add(Delegate);
}

void UInworldApiSubsystem::ClearResponseLatencyTrackerDelegate(const FOnInworldPerceivedLatencyCallback& Delegate)
{
    NO_CLIENT_RETURN(void())

    InworldSession->OnPerceivedLatencyDelegate.Remove(Delegate);
}

void UInworldApiSubsystem::LoadCharacters(const TArray<FString>& Names)
{
    NO_CLIENT_RETURN(void())
    EMPTY_ARG_RETURN(Names, void())

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
    NO_CLIENT_RETURN(void())
    EMPTY_ARG_RETURN(Names, void())

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

void UInworldApiSubsystem::SendTextMessage(const FString& AgentId, const FString& Text)
{
    NO_CLIENT_RETURN(void())
    EMPTY_ARG_RETURN(AgentId, void())
    EMPTY_ARG_RETURN(Text, void())

    InworldSession->GetClient()->SendTextMessage(AgentId, Text);
}

void UInworldApiSubsystem::SendTrigger(const FString& AgentId, const FString& Name, const TMap<FString, FString>& Params)
{
    NO_CLIENT_RETURN(void())
    EMPTY_ARG_RETURN(AgentId, void())
    EMPTY_ARG_RETURN(Name, void())

    InworldSession->GetClient()->SendTrigger(AgentId, Name, Params);
}

void UInworldApiSubsystem::SendNarrationEvent(const FString& AgentId, const FString& Content)
{
    NO_CLIENT_RETURN(void())
    EMPTY_ARG_RETURN(AgentId, void())
    EMPTY_ARG_RETURN(Content, void())

    InworldSession->GetClient()->SendNarrationEvent(AgentId, Content);
}

void UInworldApiSubsystem::SendAudioMessage(const FString& AgentId, const TArray<uint8>& InputData, const TArray<uint8>& OutputData)
{
    NO_CLIENT_RETURN(void())
    EMPTY_ARG_RETURN(InputData, void())

    InworldSession->GetClient()->SendSoundMessage(AgentId, InputData, OutputData);
}

void UInworldApiSubsystem::StartAudioSession(const FString& AgentId, FInworldAudioSessionOptions SessionOptions)
{
    NO_CLIENT_RETURN(void())
    EMPTY_ARG_RETURN(AgentId, void())

    InworldSession->GetClient()->SendAudioSessionStart(AgentId, SessionOptions);
}

void UInworldApiSubsystem::StopAudioSession(const FString& AgentId)
{
    NO_CLIENT_RETURN(void())
    EMPTY_ARG_RETURN(AgentId, void())

    InworldSession->GetClient()->SendAudioSessionStop(AgentId);
}

void UInworldApiSubsystem::ChangeScene(const FString& SceneId)
{
    NO_CLIENT_RETURN(void())
    EMPTY_ARG_RETURN(SceneId, void())

    InworldSession->SendChangeSceneEvent(SceneId);
}

EInworldConnectionState UInworldApiSubsystem::GetConnectionState() const
{
    NO_CLIENT_RETURN(EInworldConnectionState::Idle)

    return InworldSession->GetConnectionState();
}

void UInworldApiSubsystem::GetConnectionError(FString& Message, int32& Code, FInworldConnectionErrorDetails& Details)
{
    NO_CLIENT_RETURN(void())

    InworldSession->GetConnectionError(Message, Code, Details);
}

void UInworldApiSubsystem::CancelResponse(const FString& AgentId, const FString& InteractionId, const TArray<FString>& UtteranceIds)
{
    NO_CLIENT_RETURN(void())
    EMPTY_ARG_RETURN(AgentId, void())
    EMPTY_ARG_RETURN(InteractionId, void())
    EMPTY_ARG_RETURN(UtteranceIds, void())

    InworldSession->GetClient()->CancelResponse(AgentId, InteractionId, UtteranceIds);
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
    NO_SESSION_RETURN(void())
    EMPTY_ARG_RETURN(Packet, void())

    UInworldCharacter* const* InworldCharacter = InworldSession->GetRegisteredCharacters().FindByPredicate(
            [Packet](UInworldCharacter* InworldCharacter) -> bool
            {
                return InworldCharacter && InworldCharacter->GetAgentInfo().AgentId == Packet->Routing.Source.Name;
            }
    );
    if (InworldCharacter != nullptr)
    {
        (*InworldCharacter)->HandlePacket(FInworldWrappedPacket(Packet));
    }
}

#undef EMPTY_ARG_RETURN
#undef NO_SESSION_RETURN
#undef NO_CLIENT_RETURN
