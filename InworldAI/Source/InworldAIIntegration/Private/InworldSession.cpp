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
#include "InworldMacros.h"

#include "InworldAIIntegrationModule.h"

#include "Runtime/Launch/Resources/Version.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/NetDriver.h"
#include "Engine/Engine.h"

#include "Net/UnrealNetwork.h"

#define EMPTY_ARG_RETURN(Arg, Return) INWORLD_WARN_AND_RETURN_EMPTY(LogInworldAIIntegration, UInworldSession, Arg, Return)
#define NO_CLIENT_RETURN(Return) EMPTY_ARG_RETURN(Client, Return)
#define INVALID_CHARACTER_RETURN(Return) EMPTY_ARG_RETURN(Character, Return) EMPTY_ARG_RETURN(Character->GetAgentInfo().AgentId, Return)
#define INVALID_PLAYER_RETURN(Return) EMPTY_ARG_RETURN(Player, Return) EMPTY_ARG_RETURN(Player->GetConversationId(), Return)

UInworldSession::UInworldSession()
	: Client(nullptr)
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
	DOREPLIFETIME(UInworldSession, ConnectionState);
	DOREPLIFETIME(UInworldSession, RegisteredCharacters);
	DOREPLIFETIME(UInworldSession, RegisteredPlayers);
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
	Client = NewObject<UInworldClient>(this);
	OnClientPacketReceivedHandle = Client->OnPacketReceived().AddUObject(this, &UInworldSession::HandlePacket);
	OnClientConnectionStateChangedHandle = Client->OnConnectionStateChanged().AddLambda(
		[this](EInworldConnectionState InworldConnectionState) -> void
		{
			ConnectionState = InworldConnectionState;
			OnRep_ConnectionState();
		}
	);
	OnClientPerceivedLatencyHandle = Client->OnPerceivedLatency().AddLambda(
		[this](const FString& InteractionId, int32 LatencyMs) -> void
		{
			OnPerceivedLatencyDelegateNative.Broadcast(InteractionId, LatencyMs);
			OnPerceivedLatencyDelegate.Broadcast(InteractionId, LatencyMs);
		}
	);
	OnVADHandle = Client->OnVAD().AddLambda(
		[this](UObject* SessionOwner, bool bVoiceDetected) -> void
		{
			UInworldPlayer* Player = Cast<UInworldPlayer>(SessionOwner);
			if (Player)
			{
				Player->SetVoiceDetected(bVoiceDetected);
			}
			OnVADDelegate.Broadcast(Player, bVoiceDetected);
			OnVADDelegateNative.Broadcast(Player, bVoiceDetected);
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
	if (IsValid(Client))
	{
#if ENGINE_MAJOR_VERSION == 5
		Client->MarkAsGarbage();
#endif

#if ENGINE_MAJOR_VERSION == 4
		Client->MarkPendingKill();
#endif
		Client->OnPacketReceived().Remove(OnClientPacketReceivedHandle);
		Client->OnConnectionStateChanged().Remove(OnClientConnectionStateChangedHandle);
		Client->OnPerceivedLatency().Remove(OnClientPerceivedLatencyHandle);
		Client->OnVAD().Remove(OnVADHandle);
	}
	Client = nullptr;
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
		else if (Target.Type == EInworldActorType::AGENT)
		{
			if (UInworldCharacter** TargetCharacter = AgentIdToCharacter.Find(Target.Name))
			{
				(*TargetCharacter)->HandlePacket(WrappedPacket);
			}
		}
		else if (Source.Type == EInworldActorType::PLAYER)
		{
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
	EMPTY_ARG_RETURN(Character, void())

	const FString& BrainName = Character->GetAgentInfo().BrainName;

	EMPTY_ARG_RETURN(BrainName, void())

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
			Client->LoadCharacter(BrainName);
		}
	}
}

void UInworldSession::UnregisterCharacter(UInworldCharacter* Character)
{
	EMPTY_ARG_RETURN(Character, void())

	const FString& BrainName = Character->GetAgentInfo().BrainName;

	EMPTY_ARG_RETURN(BrainName, void())

	if (!ensureMsgf(BrainNameToCharacter.Contains(BrainName) && BrainNameToCharacter[BrainName] == Character, TEXT("UInworldSession::UnregisterInworldCharacter: Component mismatch for Brain: %s!"), *BrainName))
	{
		return;
	}

	AgentIdToCharacter.Remove(Character->GetAgentInfo().AgentId);
	BrainNameToCharacter.Remove(BrainName);
	RegisteredCharacters.Remove(Character);
	Client->UnloadCharacter(BrainName);
	Character->Unpossess();
}

void UInworldSession::RegisterPlayer(UInworldPlayer* Player)
{
	EMPTY_ARG_RETURN(Player, void())

	RegisteredPlayers.Add(Player);
}

void UInworldSession::UnregisterPlayer(UInworldPlayer* Player)
{
	EMPTY_ARG_RETURN(Player, void())

	RegisteredPlayers.Remove(Player);
}

void UInworldSession::StartSession(const FString& SceneId, const FInworldPlayerProfile& PlayerProfile, const FInworldAuth& Auth, const FInworldSave& Save,
	const FInworldSessionToken& SessionToken, const FInworldCapabilitySet& CapabilitySet, const FInworldPlayerSpeechOptions& SpeechOptions)
{
	NO_CLIENT_RETURN(void())

	Client->StartSession(SceneId, PlayerProfile, Auth, Save, SessionToken, CapabilitySet, SpeechOptions);
}

void UInworldSession::StopSession()
{
	UnpossessAgents();
	NO_CLIENT_RETURN(void())

	Client->StopSession();
}

void UInworldSession::PauseSession()
{
	NO_CLIENT_RETURN(void())

	Client->PauseSession();
}

void UInworldSession::ResumeSession()
{
	NO_CLIENT_RETURN(void())

	Client->ResumeSession();
}

FString UInworldSession::GetSessionId() const
{
	NO_CLIENT_RETURN({})

	return Client->GetSessionId();
}

void UInworldSession::SaveSession(FOnInworldSessionSavedCallback Callback)
{
	NO_CLIENT_RETURN(void())

	Client->SaveSession(Callback);
}

void UInworldSession::SendInteractionFeedback(const FString& InteractionId, bool bIsLike, const FString& Message)
{
	NO_CLIENT_RETURN(void())
	EMPTY_ARG_RETURN(InteractionId, void())

	Client->SendInteractionFeedback(InteractionId, bIsLike, Message);
}

void UInworldSession::LoadCharacters(const TArray<UInworldCharacter*>& Characters)
{
	NO_CLIENT_RETURN(void())
	EMPTY_ARG_RETURN(Characters, void())

	Client->LoadCharacters(Inworld::CharactersToAgentIds(Characters));
}

void UInworldSession::UnloadCharacters(const TArray<UInworldCharacter*>& Characters)
{
	NO_CLIENT_RETURN(void())
	EMPTY_ARG_RETURN(Characters, void())

	Client->UnloadCharacters(Inworld::CharactersToAgentIds(Characters));
}

void UInworldSession::LoadSavedState(const FInworldSave& Save)
{
	NO_CLIENT_RETURN(void())

	Client->LoadSavedState(Save);
}

void UInworldSession::LoadCapabilities(const FInworldCapabilitySet& CapabilitySet)
{
	NO_CLIENT_RETURN(void())

	Client->LoadCapabilities(CapabilitySet);
}

void UInworldSession::LoadPlayerProfile(const FInworldPlayerProfile& PlayerProfile)
{
	NO_CLIENT_RETURN(void())

	Client->LoadPlayerProfile(PlayerProfile);
}

void UInworldSession::SendTextMessage(UInworldCharacter* Character, const FString& Message)
{
	NO_CLIENT_RETURN(void())
	INVALID_CHARACTER_RETURN(void())
	EMPTY_ARG_RETURN(Message, void())

	auto Packet = Client->SendTextMessage(Character->GetAgentInfo().AgentId, Message).Packet;
	if (Packet.IsValid())
	{
		HandlePacket(Packet);
	}
}

void UInworldSession::SendTextMessageToConversation(UInworldPlayer* Player, const FString& Message)
{
	NO_CLIENT_RETURN(void())
	INVALID_PLAYER_RETURN(void())
	EMPTY_ARG_RETURN(Message, void())

	auto Packet = Client->SendTextMessageToConversation(Player->GetConversationId(), Message).Packet;
	if (Packet.IsValid())
	{
		HandlePacket(Packet);
	}
}

void UInworldSession::SendSoundMessage(UInworldCharacter* Character, const TArray<uint8>& InputData, const TArray<uint8>& OutputData)
{
	NO_CLIENT_RETURN(void())
	INVALID_CHARACTER_RETURN(void())
	EMPTY_ARG_RETURN(InputData, void())

	Client->SendSoundMessage(Character->GetAgentInfo().AgentId, InputData, OutputData);
}

void UInworldSession::SendSoundMessageToConversation(UInworldPlayer* Player, const TArray<uint8>& InputData, const TArray<uint8>& OutputData)
{
	NO_CLIENT_RETURN(void())
	INVALID_PLAYER_RETURN(void())
	EMPTY_ARG_RETURN(InputData, void())

	Client->SendSoundMessageToConversation(Player->GetConversationId(), InputData, OutputData);
}

void UInworldSession::SendAudioSessionStart(UInworldCharacter* Character, UInworldPlayer* Player, FInworldAudioSessionOptions SessionOptions)
{
	NO_CLIENT_RETURN(void())
	INVALID_CHARACTER_RETURN(void())

	Client->SendAudioSessionStart(Character->GetAgentInfo().AgentId, Player, SessionOptions);
}

void UInworldSession::SendAudioSessionStartToConversation(UInworldPlayer* Player, FInworldAudioSessionOptions SessionOptions)
{
	NO_CLIENT_RETURN(void())
	INVALID_PLAYER_RETURN(void())

	Client->SendAudioSessionStartToConversation(Player->GetConversationId(), Player, SessionOptions);
}

void UInworldSession::SendAudioSessionStop(UInworldCharacter* Character)
{
	NO_CLIENT_RETURN(void())
	INVALID_CHARACTER_RETURN(void())

	Client->SendAudioSessionStop(Character->GetAgentInfo().AgentId);
}

void UInworldSession::SendAudioSessionStopToConversation(UInworldPlayer* Player)
{
	NO_CLIENT_RETURN(void())
	INVALID_PLAYER_RETURN(void())

	Client->SendAudioSessionStopToConversation(Player->GetConversationId());
}

void UInworldSession::SendNarrationEvent(UInworldCharacter* Character, const FString& Content)
{
	NO_CLIENT_RETURN(void())
	INVALID_CHARACTER_RETURN(void())
	EMPTY_ARG_RETURN(Content, void())

	Client->SendNarrationEvent(Character->GetAgentInfo().AgentId, Content);
}

void UInworldSession::SendTrigger(UInworldCharacter* Character, const FString& Name, const TMap<FString, FString>& Params)
{
	NO_CLIENT_RETURN(void())
	INVALID_CHARACTER_RETURN(void())
	EMPTY_ARG_RETURN(Name, void())

	Client->SendTrigger(Character->GetAgentInfo().AgentId, Name, Params);
}

void UInworldSession::SendTriggerToConversation(UInworldPlayer* Player, const FString& Name, const TMap<FString, FString>& Params)
{
	NO_CLIENT_RETURN(void())
	INVALID_PLAYER_RETURN(void())
	EMPTY_ARG_RETURN(Name, void())

	Client->SendTriggerToConversation(Player->GetConversationId(), Name, Params);
}

void UInworldSession::SendChangeSceneEvent(const FString& SceneName)
{
	NO_CLIENT_RETURN(void())
	EMPTY_ARG_RETURN(SceneName, void())

	UnpossessAgents();
	Client->SendChangeSceneEvent(SceneName);
}

void UInworldSession::CancelResponse(UInworldCharacter* Character, const FString& InteractionId, const TArray<FString>& UtteranceIds)
{
	NO_CLIENT_RETURN(void())
	INVALID_CHARACTER_RETURN(void())
	EMPTY_ARG_RETURN(InteractionId, void())
	EMPTY_ARG_RETURN(UtteranceIds, void())

	Client->CancelResponse(Character->GetAgentInfo().AgentId, InteractionId, UtteranceIds);
}

EInworldConnectionState UInworldSession::GetConnectionState() const
{
	return ConnectionState;
}

void UInworldSession::GetConnectionError(FString& OutErrorMessage, int32& OutErrorCode, FInworldConnectionErrorDetails& OutErrorDetails) const
{
	NO_CLIENT_RETURN(void())

	Client->GetConnectionError(OutErrorMessage, OutErrorCode, OutErrorDetails);
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
		Client->LoadCharacters(BrainNames);
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

void UInworldSession::OnRep_ConnectionState()
{
	OnConnectionStateChangedDelegateNative.Broadcast(ConnectionState);
	OnConnectionStateChangedDelegate.Broadcast(ConnectionState);
}

void UInworldSession::FInworldSessionPacketVisitor::Visit(const FInworldControlEvent& Event)
{
	if (Event.Action == EInworldControlEventAction::WARNING)
	{
		UE_LOG(LogInworldAIIntegration, Warning, TEXT("%s"), *Event.Description);
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
	Session->PossessAgents(Event.AgentInfos);
}

void UInworldSession::FInworldSessionPacketVisitor::Visit(const FInworldChangeSceneEvent& Event)
{
	Session->UnpossessAgents();
	Session->PossessAgents(Event.AgentInfos);
}

#undef EMPTY_ARG_RETURN
#undef NO_CLIENT_RETURN
#undef INVALID_CHARACTER_RETURN
#undef INVALID_PLAYER_RETURN
