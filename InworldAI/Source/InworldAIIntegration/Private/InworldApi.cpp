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
    Client->LoadCharacters(Names);
}

void UInworldApiSubsystem::UnloadCharacters(const TArray<FString>& Names)
{
	Client->UnloadCharacters(Names);
}

void UInworldApiSubsystem::LoadSavedState(const TArray<uint8>& SavedState)
{
    Client->LoadSavedState(SavedState);
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
}

void UInworldApiSubsystem::UnpossessAgents()
{
    auto ComponentsToUnpossess = CharacterComponentRegistry;
    for (auto* Component : ComponentsToUnpossess)
    {
        Component->Unpossess();
    }
    CharacterComponentByAgentId.Empty();
    AgentInfoByBrain.Empty();
    bCharactersInitialized = false;
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

    TSharedPtr<FInworldPacket> Packet = Client->SendTextMessage(AgentIds, Text);
    if (Packet.IsValid())
    {
        for (auto& AgentId : AgentIds)
        {
            auto* AgentComponentPtr = CharacterComponentByAgentId.Find(AgentId);
            if (AgentComponentPtr)
            {
                (*AgentComponentPtr)->HandlePacket(Packet);
            }
        }
    }
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

    Client->SendCustomEvent(AgentIds, Name, Params);
}

void UInworldApiSubsystem::SendNarrationEvent(const FString& AgentId, const FString& Content)
{
    if (!ensureMsgf(!AgentId.IsEmpty(), TEXT("AgentId must be valid!")))
    {
        return;
    }

    Client->SendNarrationEvent(AgentId, Content);
}

void UInworldApiSubsystem::SendAudioMessage(const FString& AgentId, USoundWave* SoundWave)
{
    SendAudioMessage(TArray<FString>{ AgentId }, SoundWave);
}

void UInworldApiSubsystem::SendAudioMessage(const TArray<FString>& AgentIds, USoundWave* SoundWave)
{
    if (!ensureMsgf(AgentIds.Num() != 0, TEXT("AgentIds must be valid!")))
    {
        return;
    }

    Client->SendSoundMessage(AgentIds, SoundWave);
}

void UInworldApiSubsystem::SendAudioDataMessage(const FString& AgentId, const TArray<uint8>& Data)
{
    SendAudioDataMessage(TArray<FString>{ AgentId }, Data);
}

void UInworldApiSubsystem::SendAudioDataMessage(const TArray<FString>& AgentIds, const TArray<uint8>& Data)
{
    if (!ensureMsgf(AgentIds.Num() != 0, TEXT("AgentIdss must be valid!")))
    {
        return;
    }

    Client->SendSoundDataMessage(AgentIds, Data);
}

void UInworldApiSubsystem::SendAudioMessageWithAEC(const FString& AgentId, USoundWave* InputWave, USoundWave* OutputWave)
{
	if (!ensureMsgf(!AgentId.IsEmpty(), TEXT("AgentId must be valid!")))
	{
		return;
	}

    Client->SendSoundMessageWithEAC({ AgentId }, InputWave, OutputWave);
}

void UInworldApiSubsystem::SendAudioDataMessageWithAEC(const FString& AgentId, const TArray<uint8>& InputData, const TArray<uint8>& OutputData)
{
    SendAudioDataMessageWithAEC(TArray<FString>{ AgentId }, InputData, OutputData);
}

void UInworldApiSubsystem::SendAudioDataMessageWithAEC(const TArray<FString>& AgentIds, const TArray<uint8>& InputData, const TArray<uint8>& OutputData)
{
    if (!ensureMsgf(AgentIds.Num() != 0, TEXT("AgentIds must be valid!")))
    {
        return;
    }

    Client->SendSoundDataMessageWithEAC(AgentIds, InputData, OutputData);
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

    Client->StartAudioSession(AgentIds);
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

    Client->StopAudioSession(AgentIds);
}

void UInworldApiSubsystem::ChangeScene(const FString& SceneId)
{
    UnpossessAgents();
    Client->SendChangeSceneEvent(SceneId);
}

void UInworldApiSubsystem::GetConnectionError(FString& Message, int32& Code)
{
    Client->GetConnectionError(Message, Code);
}

Inworld::ICharacterComponent* UInworldApiSubsystem::GetCharacterComponentByAgentId(const FString& AgentId) const
{
    if (CharacterComponentByAgentId.Contains(AgentId))
    {
        return CharacterComponentByAgentId[AgentId];
    }
    return nullptr;
}

void UInworldApiSubsystem::CancelResponse(const FString& AgentId, const FString& InteractionId, const TArray<FString>& UtteranceIds)
{
	if (!ensureMsgf(!AgentId.IsEmpty(), TEXT("AgentId must be valid!")))
	{
		return;
	}

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
	auto* SourceComponentPtr = CharacterComponentByAgentId.Find(InworldPacket->Routing.Source.Name);
	if (SourceComponentPtr)
	{
		(*SourceComponentPtr)->HandlePacket(InworldPacket);
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
