/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */


#include "InworldSession.h"
#include "InworldCharacter.h"
#include "InworldPlayer.h"
#include "InworldClient.h"

#include "InworldAIIntegrationModule.h"

#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/NetDriver.h"
#include "Engine/Engine.h"

#include "Net/UnrealNetwork.h"

UInworldSession::UInworldSession()
	: InworldClient(nullptr)
	, bIsLoaded(false)
	, PacketVisitor(MakeShared<FInworldSessionPacketVisitor>(this))
{}

UInworldSession::~UInworldSession()
{}

void UInworldSession::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	if (UBlueprintGeneratedClass* BPCClass = Cast<UBlueprintGeneratedClass>(GetClass()))
	{
		BPCClass->GetLifetimeBlueprintReplicationList(OutLifetimeProps);
	}

	DOREPLIFETIME(UInworldSession, bIsLoaded);
	DOREPLIFETIME(UInworldSession, RegisteredCharacters);
}

int32 UInworldSession::GetFunctionCallspace(UFunction* Function, FFrame* Stack)
{
	if (HasAnyFlags(RF_ClassDefaultObject) || !IsSupportedForNetworking())
	{
		return GEngine->GetGlobalFunctionCallspace(Function, this, Stack);
	}

	return GetOuter()->GetFunctionCallspace(Function, Stack);
}

bool UInworldSession::CallRemoteFunction(UFunction* Function, void* Parms, FOutParmRec* OutParms, FFrame* Stack)
{
	AActor* Owner = GetTypedOuter<AActor>();
	if (UNetDriver* NetDriver = Owner->GetNetDriver())
	{
		NetDriver->ProcessRemoteFunction(Owner, Function, Parms, OutParms, Stack, this);
		return true;
	}
	return false;
}

void UInworldSession::Init()
{
	InworldClient = NewObject<UInworldClient>(this);
	OnClientPacketReceivedHandle = InworldClient->OnPacketReceived().AddUObject(this, &UInworldSession::HandlePacket);
	OnClientConnectionStateChangedHandle = InworldClient->OnConnectionStateChanged().AddLambda(
		[this](EInworldConnectionState ConnectionState) -> void
		{
			OnConnectionStateChangedDelegateNative.Broadcast(ConnectionState);
			OnConnectionStateChangedDelegate.Broadcast(ConnectionState);

			if (ConnectionState == EInworldConnectionState::Connected)
			{
				CurrentRetryConnectionTime = 1.f;
			}

			if (ConnectionState == EInworldConnectionState::Disconnected)
			{
				UWorld* World = GetWorld();
				if (!World || World->bIsTearingDown)
				{
					return;
				}
				if (CurrentRetryConnectionTime == 0.f)
				{
					ResumeSession();
				}
				else
				{
					World->GetTimerManager().SetTimer(RetryConnectionTimerHandle, this, &UInworldSession::ResumeSession, CurrentRetryConnectionTime);
				}
				CurrentRetryConnectionTime += FMath::Min(CurrentRetryConnectionTime + RetryConnectionIntervalTime, MaxRetryConnectionTime);
			}
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

void UInworldSession::Destroy()
{
	TArray<UInworldCharacter*> RegisteredCharactersCopy = RegisteredCharacters;
	for (UInworldCharacter* RegisteredCharacter : RegisteredCharactersCopy)
	{
		UnregisterCharacter(RegisteredCharacter);
	}
	if (IsValid(InworldClient))
	{
#if ENGINE_MAJOR_VERSION == 5
		InworldClient->MarkAsGarbage();
#endif

#if ENGINE_MAJOR_VERSION == 4
		InworldClient->MarkPendingKill();
#endif
		InworldClient->OnPacketReceived().Remove(OnClientPacketReceivedHandle);
		InworldClient->OnConnectionStateChanged().Remove(OnClientConnectionStateChangedHandle);
		InworldClient->OnPerceivedLatency().Remove(OnClientPerceivedLatencyHandle);
	}
	InworldClient = nullptr;
}

void UInworldSession::HandlePacket(const FInworldWrappedPacket& WrappedPacket)
{
	auto Packet = WrappedPacket.Packet;
	if (Packet.IsValid())
	{
		Packet->Accept(*PacketVisitor);

		const auto& Source = Packet->Routing.Source;
		const auto& Target = Packet->Routing.Target;
		const auto& ConversationId = Packet->Routing.ConversationId;

		if (Source.Type == EInworldActorType::AGENT)
		{
			if (UInworldCharacter** SourceCharacter = AgentIdToCharacter.Find(Source.Name))
			{
				(*SourceCharacter)->HandlePacket(WrappedPacket);
			}
		}
		if (Target.Type == EInworldActorType::AGENT)
		{
			if (UInworldCharacter** TargetCharacter = AgentIdToCharacter.Find(Target.Name))
			{
				(*TargetCharacter)->HandlePacket(WrappedPacket);
			}

			if (TArray<FString>* AgentIds = ConversationIdToAgentIds.Find(ConversationId))
			{
				for (const FString& AgentId : *AgentIds)
				{
					if (AgentId == Target.Name)
					{
						continue;
					}
					if (UInworldCharacter** TargetCharacter = AgentIdToCharacter.Find(AgentId))
					{
						(*TargetCharacter)->HandlePacket(WrappedPacket);
					}
				}
			}
		}
	}
}

void UInworldSession::RegisterCharacter(UInworldCharacter* Character)
{
	const FString& BrainName = Character->GetAgentInfo().BrainName;

	if (BrainName.IsEmpty())
	{
		return;
	}

	if (!ensureMsgf(!BrainNameToCharacter.Contains(BrainName), TEXT("UInworldSession::RegisterInworldCharacter: Character already registered for Brain: %s!"), *BrainName))
	{
		return;
	}

	RegisteredCharacters.Add(Character);
	BrainNameToCharacter.Add(BrainName, Character);

	if (bIsLoaded)
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

	if (BrainName.IsEmpty())
	{
		return;
	}

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

void UInworldSession::RegisterPlayer(UInworldPlayer* Player)
{
	RegisteredPlayers.Add(Player);
}

void UInworldSession::UnregisterPlayer(UInworldPlayer* Player)
{
	RegisteredPlayers.Remove(Player);
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

void UInworldSession::LoadCharacters(const TArray<UInworldCharacter*>& Characters)
{
	InworldClient->LoadCharacters(Inworld::CharactersToAgentIds(Characters));
}

void UInworldSession::UnloadCharacters(const TArray<UInworldCharacter*>& Characters)
{
	InworldClient->UnloadCharacters(Inworld::CharactersToAgentIds(Characters));
}

void UInworldSession::SendTextMessage(UInworldCharacter* Character, const FString& Message)
{
	auto Packet = InworldClient->SendTextMessage(Character->GetAgentInfo().AgentId, Message).Packet;
	if (Packet.IsValid())
	{
		Packet->Accept(*PacketVisitor);
	}
}

void UInworldSession::SendTextMessageToConversation(UInworldPlayer* Player, const FString& Message)
{
	auto Packet = InworldClient->SendTextMessageToConversation(Player->GetConversationId(), Message).Packet;
	if (Packet.IsValid())
	{
		Packet->Accept(*PacketVisitor);
	}
}

void UInworldSession::SendSoundMessage(UInworldCharacter* Character, const TArray<uint8>& InputData, const TArray<uint8>& OutputData)
{
	InworldClient->SendSoundMessage(Character->GetAgentInfo().AgentId, InputData, OutputData);
}

void UInworldSession::SendSoundMessageToConversation(UInworldPlayer* Player, const TArray<uint8>& InputData, const TArray<uint8>& OutputData)
{
	InworldClient->SendSoundMessageToConversation(Player->GetConversationId(), InputData, OutputData);
}

void UInworldSession::SendAudioSessionStart(UInworldCharacter* Character)
{
	InworldClient->SendAudioSessionStart(Character->GetAgentInfo().AgentId);
}

void UInworldSession::SendAudioSessionStartToConversation(UInworldPlayer* Player)
{
	InworldClient->SendAudioSessionStartToConversation(Player->GetConversationId());
}

void UInworldSession::SendAudioSessionStop(UInworldCharacter* Character)
{
	InworldClient->SendAudioSessionStop(Character->GetAgentInfo().AgentId);
}

void UInworldSession::SendAudioSessionStopToConversation(UInworldPlayer* Player)
{
	InworldClient->SendAudioSessionStopToConversation(Player->GetConversationId());
}

void UInworldSession::SendNarrationEvent(UInworldCharacter* Character, const FString& Content)
{
	InworldClient->SendNarrationEvent(Character->GetAgentInfo().AgentId, Content);
}

void UInworldSession::SendTrigger(UInworldCharacter* Character, const FString& Name, const TMap<FString, FString>& Params)
{
	InworldClient->SendTrigger(Character->GetAgentInfo().AgentId, Name, Params);
}

void UInworldSession::SendTriggerToConversation(UInworldPlayer* Player, const FString& Name, const TMap<FString, FString>& Params)
{
	InworldClient->SendTrigger(Player->GetConversationId(), Name, Params);
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

	bIsLoaded = true;
	OnRep_IsLoaded();
}

void UInworldSession::UnpossessAgents()
{
	if (!bIsLoaded)
	{
		return;
	}

	for (UInworldCharacter* Character : RegisteredCharacters)
	{
		Character->Unpossess();
	}

	AgentIdToCharacter.Empty();
	BrainNameToAgentInfo.Empty();
	bIsLoaded = false;
	OnRep_IsLoaded();
}

void UInworldSession::OnRep_IsLoaded()
{
	OnLoadedDelegateNative.Broadcast(bIsLoaded);
	OnLoadedDelegate.Broadcast(bIsLoaded);
}

void UInworldSession::OnRep_RegisteredCharacters()
{
	BrainNameToCharacter = {};
	AgentIdToCharacter = {};
	BrainNameToAgentInfo = {};
	for (UInworldCharacter* Character : RegisteredCharacters)
	{
		const FInworldAgentInfo& AgentInfo = Character->GetAgentInfo();
		BrainNameToCharacter.Add(AgentInfo.BrainName, Character);
		AgentIdToCharacter.Add(AgentInfo.AgentId, Character);
		BrainNameToAgentInfo.Add(AgentInfo.BrainName, AgentInfo);
	}
}

void UInworldSession::FInworldSessionPacketVisitor::Visit(const FInworldConversationUpdateEvent& Event)
{
	if (Event.EventType == EInworldConversationUpdateType::EVICTED)
	{
		Session->ConversationIdToAgentIds.Remove(Event.Routing.ConversationId);
	}
	else
	{
		Session->ConversationIdToAgentIds.FindOrAdd(Event.Routing.ConversationId) = Event.Agents;
	}
	UE_LOG(LogInworldAIIntegration, Log, TEXT("Conversation %s: %s, %d character(s):"),
		Event.EventType == EInworldConversationUpdateType::STARTED ? TEXT("STARTED") : Event.EventType == EInworldConversationUpdateType::EVICTED ? TEXT("EVICTED") : TEXT("UPDATED"),
		*Event.Routing.ConversationId,
		Event.Agents.Num())
	for (const auto& Agent : Event.Agents)
	{
		UE_LOG(LogInworldAIIntegration, Log, TEXT("   Agent Id: %s."), *Agent);
	}
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
