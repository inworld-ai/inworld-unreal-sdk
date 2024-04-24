/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */


#include "InworldSession.h"
#include "InworldCharacter.h"
#include "InworldClient.h"

UInworldSession::UInworldSession()
	: InworldClient(NewObject<UInworldClient>(this, TEXT("InworldClient")))
	, PacketVisitor(MakeShared<FInworldSessionPacketVisitor>(this))
{
	OnClientPacketReceivedHandle = InworldClient->OnPacketReceived().AddLambda(
		[this](const FInworldWrappedPacket& WrappedPacket) -> void
		{
			WrappedPacket.Packet->Accept(*PacketVisitor);
		}
	);
	OnClientConnectionStateChangedHandle = InworldClient->OnConnectionStateChanged().AddLambda(
		[this](EInworldConnectionState ConnectionState) -> void
		{
			OnConnectionStateChangedDelegateNative.Broadcast(ConnectionState);
			OnConnectionStateChangedDelegate.Broadcast(ConnectionState);
		}
	);
	OnClientPerceivedLatencyHandle = InworldClient->OnPerceivedLatency().AddLambda(
		[this](const FString& InteractionId, int32 LatencyMs) -> void
		{
			OnPerceivedLatencyDelegateNative.Broadcast(InteractionId, LatencyMs);
			OnPerceivedLatencyDelegate.Broadcast(InteractionId, LatencyMs);
		}
	);
}

UInworldSession::~UInworldSession()
{
	InworldClient->OnPacketReceived().Remove(OnClientPacketReceivedHandle);
	InworldClient->OnConnectionStateChanged().Remove(OnClientConnectionStateChangedHandle);
	InworldClient->OnPerceivedLatency().Remove(OnClientPerceivedLatencyHandle);
	InworldClient = nullptr;
}

void UInworldSession::RegisterCharacter(UInworldCharacter* Character)
{
	const FString& BrainName = Character->GetAgentInfo().BrainName;
	if (!ensureMsgf(!BrainNameToCharacter.Contains(BrainName), TEXT("UInworldSession::RegisterInworldCharacter: Character already registered for Brain: %s!"), *BrainName))
	{
		return;
	}

	RegisteredCharacters.Add(Character);
	BrainNameToCharacter.Add(BrainName, Character);

	if (bCharactersInitialized)
	{
		if (BrainNameToAgentInfo.Contains(BrainName))
		{
			auto AgentInfo = BrainNameToAgentInfo[BrainName];
			AgentIdToCharacter.Add(AgentInfo.AgentId, Character);
			Character->Possess(AgentInfo);
		}
		else
		{
			InworldClient->LoadCharacter(BrainName);
		}
	}
}

void UInworldSession::UnregisterCharacter(UInworldCharacter* Character)
{
	const FString& BrainName = Character->GetAgentInfo().BrainName;
	if (!ensureMsgf(BrainNameToCharacter.Contains(BrainName) && BrainNameToCharacter[BrainName] == Character, TEXT("UInworldSession::UnregisterInworldCharacter: Component mismatch for Brain: %s!"), *BrainName))
	{
		return;
	}

	AgentIdToCharacter.Remove(Character->GetAgentInfo().AgentId);
	BrainNameToCharacter.Remove(BrainName);
	RegisteredCharacters.Remove(Character);
	InworldClient->UnloadCharacter(BrainName);
	Character->Unpossess();
}

void UInworldSession::StartSession(const FString& SceneId, const FInworldPlayerProfile& PlayerProfile, const FInworldAuth& Auth, const FInworldSave& Save, const FInworldSessionToken& SessionToken, const FInworldCapabilitySet& CapabilitySet)
{
	InworldClient->StartSession(SceneId, PlayerProfile, Auth, Save, SessionToken, CapabilitySet);
}

void UInworldSession::StopSession()
{
	UnpossessAgents();
	InworldClient->StopSession();
}

TArray<FString> CharactersToAgentIds(const TArray<UInworldCharacter*>& InworldCharacters)
{
	TArray<FString> AgentIds = {};
	AgentIds.Reserve(InworldCharacters.Num());
	for (const UInworldCharacter* Character : InworldCharacters)
	{
		AgentIds.Add(Character->GetAgentInfo().AgentId);
	}
	return AgentIds;
}

void UInworldSession::LoadCharacters(const TArray<UInworldCharacter*>& Characters)
{
	InworldClient->LoadCharacters(CharactersToAgentIds(Characters));
}

void UInworldSession::UnloadCharacters(const TArray<UInworldCharacter*>& Characters)
{
	InworldClient->UnloadCharacters(CharactersToAgentIds(Characters));
}

FInworldWrappedPacket UInworldSession::BroadcastTextMessage(const TArray<UInworldCharacter*>& Characters, const FString& Message)
{
	return InworldClient->SendTextMessage(CharactersToAgentIds(Characters), Message);
}

void UInworldSession::BroadcastSoundMessage(const TArray<UInworldCharacter*>& Characters, const TArray<uint8>& InputData, const TArray<uint8>& OutputData)
{
	InworldClient->BroadcastSoundMessage(CharactersToAgentIds(Characters), InputData, OutputData);
}

void UInworldSession::BroadcastAudioSessionStart(const TArray<UInworldCharacter*>& Characters)
{
	InworldClient->BroadcastAudioSessionStart(CharactersToAgentIds(Characters));
}

void UInworldSession::BroadcastAudioSessionStop(const TArray<UInworldCharacter*>& Characters)
{
	InworldClient->BroadcastAudioSessionStop(CharactersToAgentIds(Characters));
}

void UInworldSession::SendNarrationEvent(UInworldCharacter* Character, const FString& Content)
{
	InworldClient->SendNarrationEvent(Character->GetAgentInfo().AgentId, Content);
}

void UInworldSession::BroadcastTrigger(const TArray<UInworldCharacter*>& Characters, const FString& Name, const TMap<FString, FString>& Params)
{
	InworldClient->BroadcastTrigger(CharactersToAgentIds(Characters), Name, Params);
}

void UInworldSession::SendChangeSceneEvent(const FString& SceneName)
{
	UnpossessAgents();
	InworldClient->SendChangeSceneEvent(SceneName);
}

void UInworldSession::CancelResponse(UInworldCharacter* Character, const FString& InteractionId, const TArray<FString>& UtteranceIds)
{
	InworldClient->CancelResponse(Character->GetAgentInfo().AgentId, InteractionId, UtteranceIds);
}

void UInworldSession::PossessAgents(const TArray<FInworldAgentInfo>& AgentInfos)
{
	for (const auto& AgentInfo : AgentInfos)
	{
		const FString& BrainName = AgentInfo.BrainName;
		BrainNameToAgentInfo.Add(BrainName, AgentInfo);
		if (BrainNameToCharacter.Contains(BrainName))
		{
			UInworldCharacter* Character = BrainNameToCharacter[BrainName];
			if (Character->GetAgentInfo().AgentId.IsEmpty())
			{
				AgentIdToCharacter.Add(AgentInfo.AgentId, Character);
				Character->Possess(AgentInfo);
			}
		}
		else if (BrainName != FString("__DUMMY__"))
		{
			UE_LOG(LogInworldAIIntegration, Warning, TEXT("No character found for BrainName: %s"), *BrainName);
		}
	}

	TArray<FString> BrainNames;
	for (UInworldCharacter* Character : RegisteredCharacters)
	{
		const FString& BrainName = Character->GetAgentInfo().BrainName;
		if (!BrainNameToAgentInfo.Contains(BrainName))
		{
			BrainNames.Add(BrainName);
		}
	}

	if (BrainNames.Num() > 0)
	{
		InworldClient->LoadCharacters(BrainNames);
	}

	bCharactersInitialized = true;
	OnCharactersInitializedDelegateNative.Broadcast(bCharactersInitialized);
	OnCharactersInitializedDelegate.Broadcast(bCharactersInitialized);
}

void UInworldSession::UnpossessAgents()
{
	if (!bCharactersInitialized)
	{
		return;
	}

	for (UInworldCharacter* Character : RegisteredCharacters)
	{
		Character->Unpossess();
	}

	AgentIdToCharacter.Empty();
	BrainNameToAgentInfo.Empty();
	bCharactersInitialized = false;
	OnCharactersInitializedDelegateNative.Broadcast(bCharactersInitialized);
	OnCharactersInitializedDelegate.Broadcast(bCharactersInitialized);
}

void UInworldSession::FInworldSessionPacketVisitor::Visit(const FInworldTextEvent& Event)
{
	Session->OnInworldTextEventDelegateNative.Broadcast(Event);
	Session->OnInworldTextEventDelegate.Broadcast(Event);
}

void UInworldSession::FInworldSessionPacketVisitor::Visit(const FInworldAudioDataEvent& Event)
{
	Session->OnInworldAudioEventDelegateNative.Broadcast(Event);
	Session->OnInworldAudioEventDelegate.Broadcast(Event);
}

void UInworldSession::FInworldSessionPacketVisitor::Visit(const FInworldSilenceEvent& Event)
{
	Session->OnInworldSilenceEventDelegateNative.Broadcast(Event);
	Session->OnInworldSilenceEventDelegate.Broadcast(Event);
}

void UInworldSession::FInworldSessionPacketVisitor::Visit(const FInworldControlEvent& Event)
{
	Session->OnInworldControlEventDelegateNative.Broadcast(Event);
	Session->OnInworldControlEventDelegate.Broadcast(Event);
}

void UInworldSession::FInworldSessionPacketVisitor::Visit(const FInworldEmotionEvent& Event)
{
	Session->OnInworldEmotionEventDelegateNative.Broadcast(Event);
	Session->OnInworldEmotionEventDelegate.Broadcast(Event);
}

void UInworldSession::FInworldSessionPacketVisitor::Visit(const FInworldCustomEvent& Event)
{
	Session->OnInworldCustomEventDelegateNative.Broadcast(Event);
	Session->OnInworldCustomEventDelegate.Broadcast(Event);
}

void UInworldSession::FInworldSessionPacketVisitor::Visit(const FInworldLoadCharactersEvent& Event)
{
	Session->UnpossessAgents();
	Session->PossessAgents(Event.AgentInfos);
}

void UInworldSession::FInworldSessionPacketVisitor::Visit(const FInworldChangeSceneEvent& Event)
{
	Session->PossessAgents(Event.AgentInfos);
}
