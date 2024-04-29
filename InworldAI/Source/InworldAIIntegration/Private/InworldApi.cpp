/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "InworldApi.h"
#include "InworldAIIntegrationModule.h"
#include "InworldComponentInterface.h"
#include "InworldPackets.h"
#include <Engine/Engine.h>
#include <UObject/UObjectGlobals.h>
#include "TimerManager.h"
#include "InworldAudioRepl.h"

static TAutoConsoleVariable<bool> CVarLogAllPackets(
TEXT("Inworld.Debug.LogAllPackets"), false,
TEXT("Enable/Disable logging all packets going from server")
);

#define ARG_EMPTY_RETURN(Cond, Func, Arg, Ret) if (Cond) { UE_LOG(LogInworldAIIntegration, Warning, TEXT("UInworldApiSubsystem call %s skipped: %s is empty."), TEXT(#Func), TEXT(#Arg)); return Ret; }
#define STR_EMPTY_RETURN(Func, Arg, Ret) ARG_EMPTY_RETURN(Arg.IsEmpty(), Func, Arg, Ret)
#define ARR_EMPTY_RETURN(Func, Arg, Ret) ARG_EMPTY_RETURN(Arg.Num() == 0, Func, Arg, Ret)
#define PTR_EMPTY_RETURN(Func, Arg, Ret) ARG_EMPTY_RETURN(Arg == nullptr, Func, Arg, Ret)

UInworldApiSubsystem::UInworldApiSubsystem()
    : UWorldSubsystem()
{
}

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

    FInworldCapabilitySet Capabilities;

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

    Client->Start(SceneName, PlayerProfile, Capabilities, Auth, SessionToken, FInworldSave(), Environment);
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

    Client->Start(SceneName, PlayerProfile, Capabilities, Auth, SessionToken, SavedSessionState, Environment);
}

void UInworldApiSubsystem::PauseSession()
{
    Client->Pause();
}

void UInworldApiSubsystem::ResumeSession()
{
    Client->Resume();
}

void UInworldApiSubsystem::StopSession()
{
    UnpossessAgents();

    Client->Stop();
}

void UInworldApiSubsystem::SaveSession(FOnSaveReady Delegate)
{
    Client->OnSessionSaved.BindLambda([this, Delegate](const FInworldSave& Save, bool bSuccess)
        {
            Delegate.ExecuteIfBound(Save, bSuccess);
            Client->OnSessionSaved.Unbind();
        }
    );
    Client->SaveSession();
}

void UInworldApiSubsystem::SetResponseLatencyTrackerDelegate(FResponseLatencyTrackerDelegate Delegate)
{
    Client->OnPerceivedLatency.BindLambda([this, Delegate](FString InteractionId, int32 LatancyMs)
        {
            Delegate.ExecuteIfBound(InteractionId, LatancyMs);
        });
}

void UInworldApiSubsystem::ClearResponseLatencyTrackerDelegate()
{
    Client->OnPerceivedLatency.Unbind();
}

void UInworldApiSubsystem::LoadCharacters(const TArray<FString>& Names)
{
    ARR_EMPTY_RETURN(LoadCharacters, Names,)
    Client->LoadCharacters(Names);
}

void UInworldApiSubsystem::UnloadCharacters(const TArray<FString>& Names)
{
    ARR_EMPTY_RETURN(UnloadCharacters, Names,)
	Client->UnloadCharacters(Names);
}

void UInworldApiSubsystem::LoadSavedState(const TArray<uint8>& SavedState)
{
    Client->LoadSavedState(SavedState);
}

void UInworldApiSubsystem::LoadCapabilities(const FInworldCapabilitySet& Capabilities)
{
    Client->LoadCapabilities(Capabilities);
}

void UInworldApiSubsystem::LoadPlayerProfile(const FInworldPlayerProfile& PlayerProfile)
{
    Client->LoadPlayerProfile(PlayerProfile);
}

void UInworldApiSubsystem::PossessAgents(const TArray<FInworldAgentInfo>& AgentInfos)
{
    for (const auto& AgentInfo : AgentInfos)
    {
        FString BrainName = AgentInfo.BrainName;
        AgentInfoByBrain.Add(BrainName, AgentInfo);
        if (CharacterComponentByBrainName.Contains(BrainName))
        {
            auto Component = CharacterComponentByBrainName[BrainName];
            if (!Component->IsPossessing())
            {
				CharacterComponentByAgentId.Add(AgentInfo.AgentId, Component);
				Component->Possess(AgentInfo);
				CharacterComponentByAgentId.Add(Component->GetAgentId(), Component);
            }
        }
        else if (BrainName != FString("__DUMMY__"))
        {
            UE_LOG(LogInworldAIIntegration, Warning, TEXT("No component found for BrainName: %s"), *BrainName);
        }
    }

    TArray<FString> BrainsToLoad;
    for (const auto& CharacterComponent : CharacterComponentRegistry)
    {
        auto BrainName = CharacterComponent->GetBrainName();
        if (!AgentInfoByBrain.Contains(BrainName))
        {
            BrainsToLoad.Add(BrainName);
        }
    }

    if (BrainsToLoad.Num() > 0)
    {
        Client->LoadCharacters(BrainsToLoad);
    }

    bCharactersInitialized = true;
    OnCharactersInitialized.Broadcast(bCharactersInitialized);
}

void UInworldApiSubsystem::UnpossessAgents()
{
    if (!bCharactersInitialized)
    {
        return;
    }

    auto ComponentsToUnpossess = CharacterComponentRegistry;
    for (auto* Component : ComponentsToUnpossess)
    {
        Component->Unpossess();
    }
    CharacterComponentByAgentId.Empty();
    AgentInfoByBrain.Empty();
    bCharactersInitialized = false;
    OnCharactersInitialized.Broadcast(bCharactersInitialized);
}

void UInworldApiSubsystem::RegisterCharacterComponent(Inworld::ICharacterComponent* Component)
{
    FString BrainName = Component->GetBrainName();
    if (!ensureMsgf(!CharacterComponentByBrainName.Contains(BrainName), TEXT("UInworldApiSubsystem::RegisterCharacterComponent: Component already registered for Brain: %s!"), *BrainName))
    {
        return;
    }

    CharacterComponentRegistry.Add(Component);
    CharacterComponentByBrainName.Add(BrainName, Component);

    if (bCharactersInitialized)
    {
        if (AgentInfoByBrain.Contains(BrainName))
        {
            auto AgentInfo = AgentInfoByBrain[BrainName];
            CharacterComponentByAgentId.Add(AgentInfo.AgentId, Component);
            Component->Possess(AgentInfo);
        }
        else
        {
            Client->LoadCharacters({ BrainName });
        }
    }
}

void UInworldApiSubsystem::UnregisterCharacterComponent(Inworld::ICharacterComponent* Component)
{
    auto BrainName = Component->GetBrainName();
    if (!ensureMsgf(CharacterComponentByBrainName.Contains(BrainName) && CharacterComponentByBrainName[BrainName] == Component, TEXT("UInworldApiSubsystem::UnregisterCharacterComponent: Component mismatch for Brain: %s!"), *BrainName))
    {
        return;
    }

    Component->Unpossess();
    CharacterComponentByAgentId.Remove(Component->GetAgentId());
    CharacterComponentByBrainName.Remove(BrainName);
    CharacterComponentRegistry.Remove(Component);
}

bool UInworldApiSubsystem::IsCharacterComponentRegistered(Inworld::ICharacterComponent* Component)
{
    return CharacterComponentRegistry.Contains(Component);
}

void UInworldApiSubsystem::UpdateCharacterComponentRegistrationOnClient(Inworld::ICharacterComponent* Component, const FString& NewAgentId, const FString& OldAgentId)
{
    if (CharacterComponentByAgentId.Contains(OldAgentId) && CharacterComponentByAgentId[OldAgentId] == Component)
    {
        CharacterComponentByAgentId.Remove(OldAgentId);
    }

    if (NewAgentId.IsEmpty())
    {
        CharacterComponentRegistry.Remove(Component);
    }
    else
    {
        CharacterComponentByAgentId.Add(NewAgentId, Component);
        CharacterComponentRegistry.AddUnique(Component);
    }
}

FString UInworldApiSubsystem::UpdateConversation(const FString& ConversationId, bool bIncludePlayer, const TArray<FString>& AgentIds)
{
    ARR_EMPTY_RETURN(UpdateConversation, AgentIds, {});
	UE_LOG(LogInworldAIIntegration, Log, TEXT("Update conversation %s: with %d character(s):"), *ConversationId, AgentIds.Num());
    return Client->UpdateConversation(ConversationId, bIncludePlayer, AgentIds);
}

void UInworldApiSubsystem::SendTextMessageToConversation(const FString& ConversationId, const FString& Text)
{
    STR_EMPTY_RETURN(SendTextMessageToConversation, ConversationId,);
    STR_EMPTY_RETURN(SendTextMessageToConversation, Text,);
    TSharedPtr<FInworldPacket> Packet = Client->SendTextMessageToConversation(ConversationId, Text);
    if (!Packet.IsValid())
    {
        return;
    }

    auto* AgentIds = ConversationAgentIds.Find(ConversationId);
    if (!AgentIds)
    {
        return;
    }

    for (auto& AgentId : *AgentIds)
    {
        auto* AgentComponentPtr = CharacterComponentByAgentId.Find(AgentId);
        if (AgentComponentPtr)
        {
            (*AgentComponentPtr)->HandlePacket(Packet);
        }
    }
}

void UInworldApiSubsystem::SendTriggerToConversation(const FString& ConversationId, const FString& Name,
    const TMap<FString, FString>& Params)
{
    STR_EMPTY_RETURN(SendTriggerToConversation, ConversationId,);
    STR_EMPTY_RETURN(SendTriggerToConversation, Name,);
    ARR_EMPTY_RETURN(SendTriggerToConversation, Params,);
    Client->SendCustomEventToConversation(ConversationId, Name, Params);
}

void UInworldApiSubsystem::SendAudioMessageToConversation(const FString& ConversationId, USoundWave* SoundWave)
{
    STR_EMPTY_RETURN(SendAudioMessageToConversation, ConversationId,);
    PTR_EMPTY_RETURN(SendAudioMessageToConversation, SoundWave,);
    Client->SendSoundMessageToConversation(ConversationId, SoundWave);
}

void UInworldApiSubsystem::SendAudioMessageWithAECToConversation(const FString& ConversationId, USoundWave* InputWave,
    USoundWave* OutputWave)
{
    STR_EMPTY_RETURN(SendAudioMessageWithAECToConversation, ConversationId,);
    PTR_EMPTY_RETURN(SendAudioMessageWithAECToConversation, InputWave,);
    PTR_EMPTY_RETURN(SendAudioMessageWithAECToConversation, OutputWave,);
    Client->SendSoundMessageWithAECToConversation(ConversationId, InputWave, OutputWave);
}

bool UInworldApiSubsystem::StartAudioSessionInConversation(const FString& ConversationId, const AActor* Owner)
{
    STR_EMPTY_RETURN(StartAudioSessionInConversation, ConversationId, false);
    PTR_EMPTY_RETURN(StartAudioSessionInConversation, Owner, false);
    if (AudioSessionOwner)
    {
        return false;
    }

    AudioSessionOwner = Owner;

    Client->StartAudioSessionInConversation(ConversationId);
    return true;
}

void UInworldApiSubsystem::StopAudioSessionInConversation(const FString& ConversationId)
{
    STR_EMPTY_RETURN(StopAudioSessionInConversation, ConversationId,);
    AudioSessionOwner = nullptr;
    Client->StopAudioSessionInConversation(ConversationId);
}

void UInworldApiSubsystem::SendTextMessage(const FString& AgentId, const FString& Text)
{
    STR_EMPTY_RETURN(SendTextMessage, AgentId,);
    STR_EMPTY_RETURN(SendTextMessage, Text,);
    TSharedPtr<FInworldPacket> Packet = Client->SendTextMessage(AgentId, Text);
    if (Packet.IsValid())
    {
        auto* AgentComponentPtr = CharacterComponentByAgentId.Find(AgentId);
        if (AgentComponentPtr)
        {
            (*AgentComponentPtr)->HandlePacket(Packet);
        }
    }
}

void UInworldApiSubsystem::SendTrigger(const FString& AgentId, const FString& Name, const TMap<FString, FString>& Params)
{
    STR_EMPTY_RETURN(SendTrigger, AgentId,);
    STR_EMPTY_RETURN(SendTrigger, Name,);
    ARR_EMPTY_RETURN(SendTrigger, Params,);
    Client->SendCustomEvent(AgentId, Name, Params);
}

void UInworldApiSubsystem::SendNarrationEvent(const FString& AgentId, const FString& Content)
{
    STR_EMPTY_RETURN(SendNarrationEvent, AgentId,);
    STR_EMPTY_RETURN(SendNarrationEvent, Content,);
    Client->SendNarrationEvent(AgentId, Content);
}

void UInworldApiSubsystem::SendAudioMessage(const FString& AgentId, USoundWave* SoundWave)
{
    STR_EMPTY_RETURN(SendAudioMessage, AgentId,);
    PTR_EMPTY_RETURN(SendAudioMessage, SoundWave,);
    Client->SendSoundMessage(AgentId, SoundWave);
}

void UInworldApiSubsystem::SendAudioDataMessage(const FString& AgentId, const TArray<uint8>& Data)
{
    STR_EMPTY_RETURN(SendAudioDataMessage, AgentId,);
    ARR_EMPTY_RETURN(SendAudioDataMessage, Data,);
    Client->SendSoundDataMessage(AgentId, Data);
}

void UInworldApiSubsystem::SendAudioDataMessageToConversation(const FString& ConversationId, const TArray<uint8>& Data)
{
    STR_EMPTY_RETURN(SendAudioDataMessageToConversation, ConversationId,);
    ARR_EMPTY_RETURN(SendAudioDataMessageToConversation, Data,);
    Client->SendSoundDataMessageToConversation(ConversationId, Data);
}

void UInworldApiSubsystem::SendAudioDataMessageWithAECToConversation(const FString& ConversationId,
    const TArray<uint8>& InputData, const TArray<uint8>& OutputData)
{
    STR_EMPTY_RETURN(SendAudioDataMessageWithAECToConversation, ConversationId,);
    ARR_EMPTY_RETURN(SendAudioDataMessageWithAECToConversation, InputData,);
    ARR_EMPTY_RETURN(SendAudioDataMessageWithAECToConversation, OutputData,);
    Client->SendSoundDataMessageWithAECToConversation(ConversationId, InputData, OutputData);
}

void UInworldApiSubsystem::SendAudioMessageWithAEC(const FString& AgentId, USoundWave* InputWave, USoundWave* OutputWave)
{
    STR_EMPTY_RETURN(SendAudioMessageWithAEC, AgentId,);
    PTR_EMPTY_RETURN(SendAudioMessageWithAEC, InputWave,);
    PTR_EMPTY_RETURN(SendAudioMessageWithAEC, OutputWave,);
    Client->SendSoundMessageWithAEC(AgentId, InputWave, OutputWave);
}

void UInworldApiSubsystem::SendAudioDataMessageWithAEC(const FString& AgentId, const TArray<uint8>& InputData, const TArray<uint8>& OutputData)
{
    STR_EMPTY_RETURN(SendAudioDataMessageWithAEC, AgentId,);
    ARR_EMPTY_RETURN(SendAudioDataMessageWithAEC, InputData,);
    ARR_EMPTY_RETURN(SendAudioDataMessageWithAEC, OutputData,);
    Client->SendSoundDataMessageWithAEC(AgentId, InputData, OutputData);
}

bool UInworldApiSubsystem::StartAudioSession(const FString& AgentId, const AActor* Owner)
{
    STR_EMPTY_RETURN(StartAudioSession, AgentId, false);
    PTR_EMPTY_RETURN(StartAudioSession, Owner, false);
    if (AudioSessionOwner)
    {
        return false;
    }

    AudioSessionOwner = Owner;

    Client->StartAudioSession(AgentId);
    return true;
}

void UInworldApiSubsystem::StopAudioSession(const FString& AgentId)
{
    STR_EMPTY_RETURN(StopAudioSession, AgentId,);
    AudioSessionOwner = nullptr;
    Client->StopAudioSession(AgentId);
}

void UInworldApiSubsystem::ChangeScene(const FString& SceneId)
{
    STR_EMPTY_RETURN(ChangeScene, SceneId,);
    UnpossessAgents();
    Client->SendChangeSceneEvent(SceneId);
}

void UInworldApiSubsystem::GetConnectionError(FString& Message, int32& Code)
{
    Client->GetConnectionError(Message, Code);
}

Inworld::ICharacterComponent* UInworldApiSubsystem::GetCharacterComponentByAgentId(const FString& AgentId) const
{
    STR_EMPTY_RETURN(GetCharacterComponentByAgentId, AgentId, nullptr);
    if (CharacterComponentByAgentId.Contains(AgentId))
    {
        return CharacterComponentByAgentId[AgentId];
    }
    return nullptr;
}

TArray<FString> UInworldApiSubsystem::GetConversationAgents(const FString& ConversationId) const
{
    STR_EMPTY_RETURN(GetConversationAgents, ConversationId, {});
    auto* Agents = ConversationAgentIds.Find(ConversationId);
    return Agents ? *Agents : TArray<FString>();
}

void UInworldApiSubsystem::CancelResponse(const FString& AgentId, const FString& InteractionId, const TArray<FString>& UtteranceIds)
{
    STR_EMPTY_RETURN(CancelResponse, AgentId,);
    STR_EMPTY_RETURN(CancelResponse, InteractionId,);
    ARR_EMPTY_RETURN(CancelResponse, UtteranceIds,)
    Client->CancelResponse(AgentId, InteractionId, UtteranceIds);
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

    Client = MakeShared<FInworldClient>();

    Client->OnConnectionStateChanged.BindLambda([this](EInworldConnectionState ConnectionState)
        {
            OnConnectionStateChanged.Broadcast(ConnectionState);

            if (ConnectionState == EInworldConnectionState::Connected)
            {
                CurrentRetryConnectionTime = 1.f;
            }

            if (ConnectionState == EInworldConnectionState::Disconnected)
            {
                if (CurrentRetryConnectionTime == 0.f)
                {
                    ResumeSession();
                }
                else
                {
                    GetWorld()->GetTimerManager().SetTimer(RetryConnectionTimerHandle, this, &UInworldApiSubsystem::ResumeSession, CurrentRetryConnectionTime);
                }
                CurrentRetryConnectionTime += FMath::Min(CurrentRetryConnectionTime + RetryConnectionIntervalTime, MaxRetryConnectionTime);
            }
        }
    );

    Client->OnInworldPacketReceived.BindLambda([this](TSharedPtr<FInworldPacket> Packet)
        {
            DispatchPacket(Packet);
        }
    );

    Client->Init();
}

void UInworldApiSubsystem::Deinitialize()
{
    Super::Deinitialize();
    if (Client)
    {
        Client->Destroy();
    }
    AudioSessionOwner = nullptr;
    Client.Reset();
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

void UInworldApiSubsystem::DispatchPacket(TSharedPtr<FInworldPacket> InworldPacket)
{
	if (const auto* SourceComponentPtr = CharacterComponentByAgentId.Find(InworldPacket->Routing.Source.Name))
	{
		(*SourceComponentPtr)->HandlePacket(InworldPacket);
	}

    if (InworldPacket->Routing.Source.Type == EInworldActorType::PLAYER)
    {
        auto ProcessTarget = [this, InworldPacket](const FInworldActor& TargetActor)
            {
                if (const auto* TargetComponentPtr = CharacterComponentByAgentId.Find(TargetActor.Name))
                {
                    (*TargetComponentPtr)->HandlePacket(InworldPacket);
                }
            };

        ProcessTarget(InworldPacket->Routing.Target);

        if (auto* Agents = ConversationAgentIds.Find(InworldPacket->Routing.ConversationId))
        {
            for (const auto& Agent : *Agents)
            {
                if (Agent != InworldPacket->Routing.Target.Name)
                {
                    ProcessTarget({ EInworldActorType::AGENT, Agent });
                }
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

void UInworldApiSubsystem::Visit(const FInworldControlEventConversationUpdate& Event)
{
    if (Event.EventType == EInworldConversationUpdateType::EVICTED)
    {
        ConversationAgentIds.Remove(Event.Routing.ConversationId);
    }
    else
    {
        ConversationAgentIds.FindOrAdd(Event.Routing.ConversationId) = Event.Agents;
    }
    UE_LOG(LogInworldAIIntegration, Log, TEXT("Conversation %s: %s, %d character(s):"),
        Event.EventType == EInworldConversationUpdateType::STARTED ? TEXT("STARTED") : Event.EventType == EInworldConversationUpdateType::EVICTED ? TEXT("EVICTED") : TEXT("UPDATED"),
        *Event.Routing.ConversationId,
        Event.Agents.Num())
	for (const auto& Agent : Event.Agents)
	{
		UE_LOG(LogInworldAIIntegration, Log, TEXT("   Agent Id: %s."), *Agent);
	}
    OnConversationUpdate.Broadcast(Event.Routing.ConversationId, Event.EventType, Event.Agents, Event.bIncludePlayer);
}

void UInworldApiSubsystem::ReplicateAudioEventFromServer(FInworldAudioDataEvent& Packet)
{
    if (AudioRepl)
    {
        AudioRepl->ReplicateAudioEvent(Packet);
    }
}

void UInworldApiSubsystem::HandleAudioEventOnClient(TSharedPtr<FInworldAudioDataEvent> Packet)
{
    DispatchPacket(Packet);
}
