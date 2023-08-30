/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "InworldApi.h"
#include "NDK/Proto/ProtoDisableWarning.h"
#include "InworldComponentInterface.h"
#include "InworldUtils.h"
#include <Engine/Engine.h>
#include <UObject/UObjectGlobals.h>
#include <Interfaces/IPluginManager.h>
#include "TimerManager.h"
#include "NDK/Client.h"
#include "NDK/Utils/Log.h"
#include "InworldAudioRepl.h"

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

static TAutoConsoleVariable<bool> CVarLogAllPackets(
TEXT("Inworld.Debug.LogAllPackets"), false,
TEXT("Enable/Disable logging all packets going from server")
);

const FString DefaultAuthUrl = "api-studio.inworld.ai";
const FString DefaultTargetUrl = "api-engine.inworld.ai:443";

UInworldApiSubsystem::UInworldApiSubsystem()
    : UWorldSubsystem()
{
}

void UInworldApiSubsystem::StartSession(const FString& SceneName, const FString& PlayerName, const FString& ApiKey, const FString& ApiSecret, const FString& AuthUrlOverride, const FString& TargetUrlOverride, const FString& Token, int64 TokenExpirationTime, const FString& SessionId)
{
    if (!ensure(GetWorld()->GetNetMode() < NM_Client))
    {
        Inworld::LogError("UInworldApiSubsystem::StartSession shouldn't be called on client");
        return;
    }

    if (ApiKey.IsEmpty())
    {
        Inworld::LogError("Can't Start Session, ApiKey is empty");
        return;
    }
    if (ApiSecret.IsEmpty())
    {
        Inworld::LogError("Can't Start Session, ApiSecret is empty");
        return;
    }
    
    Inworld::ClientOptions Options;
    Options.ServerUrl = TCHAR_TO_UTF8(*(!TargetUrlOverride.IsEmpty() ? TargetUrlOverride : DefaultTargetUrl));
    Options.SceneName = TCHAR_TO_UTF8(*SceneName);
    Options.ApiKey = TCHAR_TO_UTF8(*ApiKey);
    Options.ApiSecret = TCHAR_TO_UTF8(*ApiSecret);
    Options.PlayerName = TCHAR_TO_UTF8(*PlayerName);

    Inworld::SessionInfo Info;
    Info.Token = TCHAR_TO_UTF8(*Token);
    Info.ExpirationTime = TokenExpirationTime;
    Info.SessionId = TCHAR_TO_UTF8(*SessionId);

    const FInworldCapabilitySet Capabilities;

    Options.Capabilities.Animations = Capabilities.Animations;
    Options.Capabilities.Text = Capabilities.Text;
    Options.Capabilities.Audio = Capabilities.Audio;
    Options.Capabilities.Emotions = Capabilities.Emotions;
    Options.Capabilities.Gestures = Capabilities.Gestures;
    Options.Capabilities.Interruptions = Capabilities.Interruptions;
    Options.Capabilities.Triggers = Capabilities.Triggers;
    Options.Capabilities.EmotionStreaming = Capabilities.EmotionStreaming;
    Options.Capabilities.SilenceEvents = Capabilities.SilenceEvents;
    Options.Capabilities.PhonemeInfo = Capabilities.PhonemeInfo;
    Options.Capabilities.LoadSceneInSession = Capabilities.LoadSceneInSession;

    Client->StartClient(Options, Info,
        [this](const auto& AgentInfos)
        {
            PossessAgents(AgentInfos);
        });
}

void UInworldApiSubsystem::StartSession_V2(const FString& SceneName, const FInworldPlayerProfile& PlayerProfile, const FInworldCapabilitySet& Capabilities, const FInworldAuth& Auth, const FInworldSessionToken& SessionToken, const FInworldEnvironment& Environment, FString UniqueUserIdOverride, FInworldSave SavedSessionState)
{
    if (!ensure(GetWorld()->GetNetMode() < NM_Client))
    {
        Inworld::LogError("UInworldApiSubsystem::StartSession shouldn't be called on client");
        return;
    }

    if (Auth.ApiKey.IsEmpty())
    {
        Inworld::LogError("Can't Start Session, ApiKey is empty");
        return;
    }
    if (Auth.ApiSecret.IsEmpty())
    {
        Inworld::LogError("Can't Start Session, ApiSecret is empty");
        return;
    }

    Inworld::ClientOptions Options;
    Options.ServerUrl = TCHAR_TO_UTF8(*(!Environment.TargetUrl.IsEmpty() ? Environment.TargetUrl : DefaultTargetUrl));
    Options.SceneName = TCHAR_TO_UTF8(*SceneName);
    Options.ApiKey = TCHAR_TO_UTF8(*Auth.ApiKey);
    Options.ApiSecret = TCHAR_TO_UTF8(*Auth.ApiSecret);
    Options.PlayerName = TCHAR_TO_UTF8(*PlayerProfile.Name);
    if (!UniqueUserIdOverride.IsEmpty())
    {
        Options.UserId = TCHAR_TO_UTF8(*UniqueUserIdOverride);
    }

    Options.UserSettings.Profile.Fields.reserve(PlayerProfile.Fields.Num());
    for (const auto& ProfileField : PlayerProfile.Fields)
    {
        Inworld::UserSettings::PlayerProfile::PlayerField PlayerField;
        PlayerField.Id = TCHAR_TO_UTF8(*ProfileField.Key);
        PlayerField.Value = TCHAR_TO_UTF8(*ProfileField.Value);
        Options.UserSettings.Profile.Fields.push_back(PlayerField);
    }

    Options.Capabilities.Animations = Capabilities.Animations;
    Options.Capabilities.Text = Capabilities.Text;
    Options.Capabilities.Audio = Capabilities.Audio;
    Options.Capabilities.Emotions = Capabilities.Emotions;
    Options.Capabilities.Gestures = Capabilities.Gestures;
    Options.Capabilities.Interruptions = Capabilities.Interruptions;
    Options.Capabilities.Triggers = Capabilities.Triggers;
    Options.Capabilities.EmotionStreaming = Capabilities.EmotionStreaming;
    Options.Capabilities.SilenceEvents = Capabilities.SilenceEvents;
    Options.Capabilities.PhonemeInfo = Capabilities.PhonemeInfo;
    Options.Capabilities.LoadSceneInSession = Capabilities.LoadSceneInSession;

    Inworld::SessionInfo Info;
    Info.Token = TCHAR_TO_UTF8(*SessionToken.Token);
    Info.ExpirationTime = SessionToken.ExpirationTime;
    Info.SessionId = TCHAR_TO_UTF8(*SessionToken.SessionId);
    if (SavedSessionState.Data.Num() != 0)
    {
        Info.SessionSavedState.resize(SavedSessionState.Data.Num());
        FMemory::Memcpy((uint8*)Info.SessionSavedState.data(), (uint8*)SavedSessionState.Data.GetData(), Info.SessionSavedState.size());
    }

    Client->StartClient(Options, Info,
        [this](const auto& AgentInfos)
        {
            PossessAgents(AgentInfos);
        });
}

void UInworldApiSubsystem::PauseSession()
{
    Client->PauseClient();
}

void UInworldApiSubsystem::ResumeSession()
{
    Client->ResumeClient();
}

void UInworldApiSubsystem::StopSession()
{
    UnpossessAgents();

    Client->StopClient();
}

void UInworldApiSubsystem::SaveSession(FOnSaveReady Delegate)
{
    Client->SaveSessionState([Delegate](std::string Data, bool bSuccess)
		{
			FInworldSave Save;
            if (!bSuccess)
            {
                Inworld::LogError("UInworldApiSubsystem::SaveSession error");
                Delegate.ExecuteIfBound(Save, false);
                return;
            }

            Save.Data.SetNumUninitialized(Data.size());
            FMemory::Memcpy((uint8*)Save.Data.GetData(), (uint8*)Data.data(), Save.Data.Num());

            Delegate.ExecuteIfBound(Save, true);
        });
}

void UInworldApiSubsystem::SetResponseLatencyTrackerDelegate(FResponseLatencyTrackerDelegate Delegate)
{
    Client->SetPerceivedLatencyTrackerCallback([Delegate](const std::string& InteractionId, uint32_t LatancyMs)
        {
            Delegate.ExecuteIfBound(UTF8_TO_TCHAR(InteractionId.c_str()), LatancyMs);
        });
}

void UInworldApiSubsystem::ClearResponseLatencyTrackerDelegate()
{
    Client->ClearPerceivedLatencyTrackerCallback();
}

void UInworldApiSubsystem::PossessAgents(const std::vector<Inworld::AgentInfo>& AgentInfos)
{
    for (const auto& AgentInfo : AgentInfos)
    {
        FString BrainName = UTF8_TO_TCHAR(AgentInfo.BrainName.c_str());
        AgentInfoByBrain.Add(BrainName, AgentInfo);
        if (CharacterComponentByBrainName.Contains(BrainName))
        {
            auto Component = CharacterComponentByBrainName[BrainName];
            CharacterComponentByAgentId.Add(UTF8_TO_TCHAR(AgentInfo.AgentId.c_str()), Component);
            Component->Possess(AgentInfo);
            CharacterComponentByAgentId.Add(Component->GetAgentId(), Component);
        }
        else if (BrainName != FString("__DUMMY__"))
        {
            Inworld::LogWarning("No component found for BrainName: %s", *BrainName);
        }
    }

    for (const auto& CharacterComponent : CharacterComponentRegistry)
    {
        auto BrainName = CharacterComponent->GetBrainName();
        if (!AgentInfoByBrain.Contains(BrainName))
        {
            Inworld::LogError("No agent found for BrainName: %s", *BrainName);
        }
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
            CharacterComponentByAgentId.Add(UTF8_TO_TCHAR(AgentInfo.AgentId.c_str()), Component);
            Component->Possess(AgentInfo);
        }
        else
        {
            Inworld::LogError("No agent found for BrainName: %s", *BrainName);
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
    if (!ensureMsgf(!AgentId.IsEmpty(), TEXT("AgentId must be valid!")))
    {
        return;
    }

    auto Packet = Client->SendTextMessage(TCHAR_TO_UTF8(*AgentId), TCHAR_TO_UTF8(*Text));
    auto* AgentComponentPtr = CharacterComponentByAgentId.Find(AgentId);
    if (AgentComponentPtr)
    {
		InworldPacketCreator PacketCreator;
		Packet->Accept(PacketCreator);

        (*AgentComponentPtr)->HandlePacket(PacketCreator.GetPacket());
    }
}

void UInworldApiSubsystem::SendTrigger(FString AgentId, const FString& Name, const TMap<FString, FString>& Params)
{
	if (!ensureMsgf(!AgentId.IsEmpty(), TEXT("AgentId must be valid!")))
	{
		return;
	}

    std::unordered_map<std::string, std::string> params;

    for (const TPair<FString, FString>& Param : Params)
    {
        params.insert(std::make_pair<std::string, std::string>(TCHAR_TO_UTF8(*Param.Key), TCHAR_TO_UTF8(*Param.Value)));
    }

    Client->SendCustomEvent(TCHAR_TO_UTF8(*AgentId), TCHAR_TO_UTF8(*Name), params);
}

void UInworldApiSubsystem::SendAudioMessage(const FString& AgentId, USoundWave* SoundWave)
{
	if (!ensureMsgf(!AgentId.IsEmpty(), TEXT("AgentId must be valid!")))
	{
		return;
	}

    std::string Data;
    if (ensure(SoundWave && Inworld::Utils::SoundWaveToString(SoundWave, Data)))
    {
        SendAudioDataMessage(AgentId, Data);
    }
}

void UInworldApiSubsystem::SendAudioDataMessage(const FString& AgentId, const std::string& Data)
{
	if (!ensureMsgf(!AgentId.IsEmpty(), TEXT("AgentId must be valid!")))
	{
		return;
	}

    Client->SendSoundMessage(TCHAR_TO_UTF8(*AgentId), Data);
}

void UInworldApiSubsystem::SendAudioDataMessageWithAEC(const FString& AgentId, USoundWave* InputWave, USoundWave* OutputWave)
{
	if (!ensureMsgf(!AgentId.IsEmpty(), TEXT("AgentId must be valid!")))
	{
		return;
	}

	std::vector<int16_t> InputData, OutputData;
	if (ensure(Inworld::Utils::SoundWaveToVec(InputWave, InputData) && Inworld::Utils::SoundWaveToVec(OutputWave, OutputData)))
	{
		Client->SendSoundMessageWithAEC(TCHAR_TO_UTF8(*AgentId), InputData, OutputData);
	}
}

void UInworldApiSubsystem::SendAudioDataMessageWithAEC(const FString& AgentId, const std::vector<int16_t>& InputData, const std::vector<int16_t>& OutputData)
{
	if (!ensureMsgf(!AgentId.IsEmpty(), TEXT("AgentId must be valid!")))
	{
		return;
	}

    Client->SendSoundMessageWithAEC(TCHAR_TO_UTF8(*AgentId), InputData, OutputData);
}

void UInworldApiSubsystem::StartAudioSession(const FString& AgentId)
{
	if (!ensureMsgf(!AgentId.IsEmpty(), TEXT("AgentId must be valid!")))
	{
		return;
	}

    Client->StartAudioSession(TCHAR_TO_UTF8(*AgentId));
}

void UInworldApiSubsystem::StopAudioSession(const FString& AgentId)
{
	if (!ensureMsgf(!AgentId.IsEmpty(), TEXT("AgentId must be valid!")))
	{
		return;
	}

    Client->StopAudioSession(TCHAR_TO_UTF8(*AgentId));
}

void UInworldApiSubsystem::ChangeScene(const FString& SceneId)
{
    UnpossessAgents();
    Client->SendChangeSceneEvent(TCHAR_TO_UTF8(*SceneId));
}

void UInworldApiSubsystem::GetConnectionError(FString& Message, int32& Code)
{
    std::string OutErrorString;
    Client->GetConnectionError(OutErrorString, Code);
    Message = FString(UTF8_TO_TCHAR(OutErrorString.c_str()));
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

    std::vector<std::string> IDs;
    IDs.reserve(UtteranceIds.Num());
    for (auto& Id : UtteranceIds)
    {
        IDs.push_back(TCHAR_TO_UTF8(*Id));
    }
    Client->CancelResponse(TCHAR_TO_UTF8(*AgentId), TCHAR_TO_UTF8(*InteractionId), IDs);
}

void UInworldApiSubsystem::CancelResponse(const FString& AgentId, const std::string& InteractionId, const std::vector<std::string>& UtteranceIds)
{
	if (!ensureMsgf(!AgentId.IsEmpty(), TEXT("AgentId must be valid!")))
	{
		return;
	}
    Client->CancelResponse(TCHAR_TO_UTF8(*AgentId), InteractionId, UtteranceIds);
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

    std::string ClientVer;
	TSharedPtr<IPlugin> InworldAIPlugin = IPluginManager::Get().FindPlugin("InworldAI");
	if (ensure(InworldAIPlugin.IsValid()))
	{
        ClientVer = TCHAR_TO_UTF8(*InworldAIPlugin.Get()->GetDescriptor().VersionName);
	}

    Client = std::make_shared<Inworld::FClient>();
    Client->SetSelfWeakPtr(Client);

    Client->InitClient(
        "unreal",
        ClientVer,
        [this](Inworld::ClientBase::ConnectionState ConnectionState)
        {
            OnConnectionStateChanged.Broadcast(static_cast<EInworldConnectionState>(ConnectionState));

            if (ConnectionState == Inworld::ClientBase::ConnectionState::Connected)
            {
                CurrentRetryConnectionTime = 0.f;
            }

            if (ConnectionState == Inworld::ClientBase::ConnectionState::Disconnected)
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
        },
        [this](std::shared_ptr<Inworld::Packet> Packet)
        {
            if (!Packet)
            {
                return;
            }

            InworldPacketCreator PacketCreator;
            Packet->Accept(PacketCreator);
            auto InworldPacket = PacketCreator.GetPacket();
#if !UE_BUILD_SHIPPING
            if (CVarLogAllPackets.GetValueOnGameThread())
            {
                Inworld::Log("PACKET: %s", *InworldPacket->ToDebugString());
            }
#endif
            DispatchPacket(InworldPacket);
        });
}

void UInworldApiSubsystem::Deinitialize()
{
    Super::Deinitialize();
    Client->DestroyClient();
    Client.reset();
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

void UInworldApiSubsystem::DispatchPacket(std::shared_ptr<FInworldPacket> InworldPacket)
{
	auto* SourceComponentPtr = CharacterComponentByAgentId.Find(InworldPacket->Routing.Source.Name);
	if (SourceComponentPtr)
	{
		(*SourceComponentPtr)->HandlePacket(InworldPacket);
	}

	auto* TargetComponentPtr = CharacterComponentByAgentId.Find(InworldPacket->Routing.Target.Name);
	if (TargetComponentPtr)
	{
		(*TargetComponentPtr)->HandlePacket(InworldPacket);
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

void UInworldApiSubsystem::ReplicateAudioEventFromServer(FInworldAudioDataEvent& Packet)
{
    if (AudioRepl)
    {
        AudioRepl->ReplicateAudioEvent(Packet);
    }
}

void UInworldApiSubsystem::HandleAudioEventOnClient(std::shared_ptr<FInworldAudioDataEvent> Packet)
{
    DispatchPacket(Packet);
}
