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

void UInworldApiSubsystem::StartSession(const FString& SceneName, const FString& PlayerName, const FString& ApiKey, const FString& ApiSecret, const FString& AuthUrlOverride, const FString& TargetUrlOverride, const FString& Token, int64 TokenExpirationTime, const FString& SessionId)
{
    NO_CLIENT_RETURN(void())
    EMPTY_ARG_RETURN(ApiKey, void())
    EMPTY_ARG_RETURN(ApiSecret, void())

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

    InworldSession->GetClient()->StartSession(SceneName, PlayerProfile, Auth, FInworldSave(), SessionToken, {});
}

void UInworldApiSubsystem::StartSession_V2(const FString& SceneName, const FInworldPlayerProfile& PlayerProfile, const FInworldCapabilitySet& Capabilities, const FInworldAuth& Auth, const FInworldSessionToken& SessionToken, const FInworldEnvironment& Environment, FString UniqueUserIdOverride, FInworldSave SavedSessionState)
{
    NO_CLIENT_RETURN(void())

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

    InworldSession->GetClient()->StartSession(SceneName, PlayerProfile, Auth, SavedSessionState, SessionToken, Capabilities);
}

void UInworldApiSubsystem::PauseSession()
{
    NO_CLIENT_RETURN(void())

    InworldSession->GetClient()->PauseSession();
}

void UInworldApiSubsystem::ResumeSession()
{
    NO_CLIENT_RETURN(void())

    InworldSession->GetClient()->ResumeSession();
}

void UInworldApiSubsystem::StopSession()
{
    NO_CLIENT_RETURN(void())

    InworldSession->GetClient()->StopSession();
}

void UInworldApiSubsystem::SaveSession(FOnInworldSessionSavedCallback Callback)
{
    NO_CLIENT_RETURN(void())

    InworldSession->GetClient()->SaveSession(Callback);
}

void UInworldApiSubsystem::SetResponseLatencyTrackerDelegate(const FOnInworldPerceivedLatencyCallback& Delegate)
{
    NO_CLIENT_RETURN(void())

    InworldSession->GetClient()->OnPerceivedLatencyDelegate.Add(Delegate);
}

void UInworldApiSubsystem::ClearResponseLatencyTrackerDelegate(const FOnInworldPerceivedLatencyCallback& Delegate)
{
    NO_CLIENT_RETURN(void())

    InworldSession->GetClient()->OnPerceivedLatencyDelegate.Remove(Delegate);
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

void UInworldApiSubsystem::LoadSavedState(const FInworldSave& SavedState)
{
    NO_CLIENT_RETURN(void())

    InworldSession->GetClient()->LoadSavedState(SavedState);
}

void UInworldApiSubsystem::LoadCapabilities(const FInworldCapabilitySet& Capabilities)
{
    NO_CLIENT_RETURN(void())

    InworldSession->GetClient()->LoadCapabilities(Capabilities);
}

void UInworldApiSubsystem::LoadPlayerProfile(const FInworldPlayerProfile& PlayerProfile)
{
    NO_CLIENT_RETURN(void())

    InworldSession->GetClient()->LoadPlayerProfile(PlayerProfile);
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

void UInworldApiSubsystem::StartAudioSession(const FString& AgentId, UInworldPlayer* Player, EInworldMicrophoneMode MicrophoneMode/* = EInworldMicrophoneMode::OPEN_MIC*/)
{
    NO_CLIENT_RETURN(void())
    EMPTY_ARG_RETURN(AgentId, void())

    InworldSession->GetClient()->SendAudioSessionStart(AgentId, Cast<UObject>(Player), MicrophoneMode);
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

    InworldSession->GetClient()->SendChangeSceneEvent(SceneId);
}

EInworldConnectionState UInworldApiSubsystem::GetConnectionState() const
{
    NO_CLIENT_RETURN(EInworldConnectionState::Idle)

    return InworldSession->GetConnectionState();
}

void UInworldApiSubsystem::GetConnectionError(FString& Message, int32& Code)
{
    NO_CLIENT_RETURN(void())

    InworldSession->GetConnectionError(Message, Code);
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
