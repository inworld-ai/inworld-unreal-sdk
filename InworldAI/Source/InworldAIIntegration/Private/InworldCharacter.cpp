/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */


#include "InworldCharacter.h"
#include "InworldSession.h"
#include "InworldPlayer.h"
#include "InworldMacros.h"

#include "InworldAIIntegrationModule.h"

#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/NetDriver.h"
#include "Engine/Engine.h"

#include "Net/UnrealNetwork.h"

#define EMPTY_ARG_RETURN(Arg, Return) INWORLD_WARN_AND_RETURN_EMPTY(LogInworldAIIntegration, UInworldCharacter, Arg, Return)
#define NO_SESSION_RETURN(Return) EMPTY_ARG_RETURN(Session, Return)

UInworldCharacter::UInworldCharacter()
	: Super()
	, PacketVisitor(MakeShared<FInworldCharacterPacketVisitor>(this))
{}

UInworldCharacter::~UInworldCharacter()
{}

void UInworldCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	if (UBlueprintGeneratedClass* BPCClass = Cast<UBlueprintGeneratedClass>(GetClass()))
	{
		BPCClass->GetLifetimeBlueprintReplicationList(OutLifetimeProps);
	}

	DOREPLIFETIME(UInworldCharacter, Session);
	DOREPLIFETIME(UInworldCharacter, AgentInfo);
	DOREPLIFETIME(UInworldCharacter, TargetPlayer);
}

int32 UInworldCharacter::GetFunctionCallspace(UFunction* Function, FFrame* Stack)
{
	if (HasAnyFlags(RF_ClassDefaultObject) || !IsSupportedForNetworking())
	{
		return GEngine->GetGlobalFunctionCallspace(Function, this, Stack);
	}

	return GetOuter()->GetFunctionCallspace(Function, Stack);
}

bool UInworldCharacter::CallRemoteFunction(UFunction* Function, void* Parms, FOutParmRec* OutParms, FFrame* Stack)
{
	AActor* Owner = GetTypedOuter<AActor>();
	if (UNetDriver* NetDriver = Owner->GetNetDriver())
	{
		NetDriver->ProcessRemoteFunction(Owner, Function, Parms, OutParms, Stack, this);
		return true;
	}
	return false;
}

void UInworldCharacter::HandlePacket(const FInworldWrappedPacket& WrappedPacket)
{
	auto Packet = WrappedPacket.Packet;
	if (Packet.IsValid())
	{
		Packet->Accept(*PacketVisitor);
	}
}

void UInworldCharacter::SetSession(UInworldSession* InSession)
{
	if (Session == InSession)
	{
		return;
	}

	if (Session && !AgentInfo.BrainName.IsEmpty())
	{
		Session->UnregisterCharacter(this);
	}

	Session = InSession;

	if (Session && !AgentInfo.BrainName.IsEmpty())
	{
		Session->RegisterCharacter(this);
	}
}

void UInworldCharacter::SendTextMessage(const FString& Text)
{
	NO_SESSION_RETURN(void())
	EMPTY_ARG_RETURN(AgentInfo.AgentId, void())
	EMPTY_ARG_RETURN(Text, void())

	Session->SendTextMessage(this, Text);
}

void UInworldCharacter::SendTrigger(const FString& Name, const TMap<FString, FString>& Params)
{
	NO_SESSION_RETURN(void())
	EMPTY_ARG_RETURN(AgentInfo.AgentId, void())
	EMPTY_ARG_RETURN(Name, void())

	Session->SendTrigger(this, Name, Params);
}

void UInworldCharacter::SendNarrationEvent(const FString& Content)
{
	NO_SESSION_RETURN(void())
	EMPTY_ARG_RETURN(AgentInfo.AgentId, void())
	EMPTY_ARG_RETURN(Content, void())

	Session->SendNarrationEvent(this, Content);
}

void UInworldCharacter::SendAudioSessionStart(UInworldPlayer* Player, FInworldAudioSessionOptions Playback)
{
	NO_SESSION_RETURN(void())
	EMPTY_ARG_RETURN(AgentInfo.AgentId, void())

	Session->SendAudioSessionStart(this, Player, Playback);
}

void UInworldCharacter::SendAudioSessionStop()
{
	NO_SESSION_RETURN(void())
	EMPTY_ARG_RETURN(AgentInfo.AgentId, void())

	Session->SendAudioSessionStop(this);
}

void UInworldCharacter::SendSoundMessage(const TArray<uint8>& InputData, const TArray<uint8>& OutputData)
{
	NO_SESSION_RETURN(void())
	EMPTY_ARG_RETURN(AgentInfo.AgentId, void())
	EMPTY_ARG_RETURN(InputData, void())

	Session->SendSoundMessage(this, InputData, OutputData);
}

void UInworldCharacter::CancelResponse(const FString& InteractionId, const TArray<FString>& UtteranceIds)
{
	NO_SESSION_RETURN(void())
	EMPTY_ARG_RETURN(AgentInfo.AgentId, void())
	EMPTY_ARG_RETURN(InteractionId, void())
	EMPTY_ARG_RETURN(UtteranceIds, void())

	Session->CancelResponse(this, InteractionId, UtteranceIds);
}

TScriptInterface<IInworldCharacterOwnerInterface> UInworldCharacter::GetInworldCharacterOwner()
{
	if (!ensureMsgf(GetOuter()->Implements<UInworldCharacterOwnerInterface>(), TEXT("UInworldCharacter outer must implement IInworldCharacterOwnerInterface!")))
	{
		return nullptr;
	}
	return TScriptInterface<IInworldCharacterOwnerInterface>(GetOuter());
}

void UInworldCharacter::SetBrainName(const FString& BrainName)
{
	if (!AgentInfo.BrainName.IsEmpty())
	{
		if (Session != nullptr)
		{
			Session->UnregisterCharacter(this);
		}
	}
	AgentInfo.BrainName = BrainName;
	if (!BrainName.IsEmpty())
	{
		if (Session != nullptr)
		{
			Session->RegisterCharacter(this);
		}
	}
}

void UInworldCharacter::Possess(const FInworldAgentInfo& InAgentInfo)
{
	if (InAgentInfo.AgentId.IsEmpty())
	{
		Unpossess();
		return;
	}
	if (AgentInfo.AgentId != InAgentInfo.AgentId)
	{
		UInworldPlayer* CachedTargetPlayer = TargetPlayer;
		Unpossess();
		AgentInfo = InAgentInfo;
		OnPossessedDelegateNative.Broadcast(true);
		OnPossessedDelegate.Broadcast(true);
		if(CachedTargetPlayer != nullptr)
		{
			CachedTargetPlayer->AddTargetCharacter(this);
		}
	}
}

void UInworldCharacter::Unpossess()
{
	if (IsPossessed())
	{
		if (TargetPlayer != nullptr)
		{
			TargetPlayer->RemoveTargetCharacter(this);
		}
		OnPossessedDelegateNative.Broadcast(false);
		OnPossessedDelegate.Broadcast(false);
		AgentInfo.AgentId = {};
		AgentInfo.GivenName = {};
	}
}

void UInworldCharacter::SetTargetPlayer(UInworldPlayer* Player)
{
	if (Player != TargetPlayer)
	{
		UInworldPlayer* Old = TargetPlayer;
		ClearTargetPlayer();
		TargetPlayer = Player;
		OnRep_TargetPlayer(Old);
	}
}

void UInworldCharacter::ClearTargetPlayer()
{
	if (TargetPlayer != nullptr)
	{
		UInworldPlayer* Old = TargetPlayer;
		TargetPlayer = nullptr;
		OnRep_TargetPlayer(Old);
	}
}

void UInworldCharacter::OnRep_TargetPlayer(UInworldPlayer* OldTargetPlayer)
{
	OnTargetPlayerChangedDelegateNative.Broadcast();
	OnTargetPlayerChangedDelegate.Broadcast();

	if (OldTargetPlayer && OnVADHandle.IsValid())
	{
		OldTargetPlayer->OnVoiceDetection().Remove(OnVADHandle);
	}
	if (TargetPlayer)
	{
		OnVADHandle = TargetPlayer->OnVoiceDetection().AddLambda(
			[this](bool bVoiceDetected) -> void
		{
			GetInworldCharacterOwner()->HandleTargetPlayerVoiceDetection(bVoiceDetected);
		});
	}
}

void UInworldCharacter::FInworldCharacterPacketVisitor::Visit(const FInworldTextEvent& Event)
{
	Character->OnInworldTextEventDelegateNative.Broadcast(Event);
	Character->OnInworldTextEventDelegate.Broadcast(Event);
}

void UInworldCharacter::FInworldCharacterPacketVisitor::Visit(const FInworldAudioDataEvent& Event)
{
	Character->OnInworldAudioEventDelegateNative.Broadcast(Event);
	Character->OnInworldAudioEventDelegate.Broadcast(Event);
}

void UInworldCharacter::FInworldCharacterPacketVisitor::Visit(const FInworldSilenceEvent& Event)
{
	Character->OnInworldSilenceEventDelegateNative.Broadcast(Event);
	Character->OnInworldSilenceEventDelegate.Broadcast(Event);
}

void UInworldCharacter::FInworldCharacterPacketVisitor::Visit(const FInworldControlEvent& Event)
{
	Character->OnInworldControlEventDelegateNative.Broadcast(Event);
	Character->OnInworldControlEventDelegate.Broadcast(Event);
}

void UInworldCharacter::FInworldCharacterPacketVisitor::Visit(const FInworldEmotionEvent& Event)
{
	Character->OnInworldEmotionEventDelegateNative.Broadcast(Event);
	Character->OnInworldEmotionEventDelegate.Broadcast(Event);
}

void UInworldCharacter::FInworldCharacterPacketVisitor::Visit(const FInworldCustomEvent& Event)
{
	Character->OnInworldCustomEventDelegateNative.Broadcast(Event);
	Character->OnInworldCustomEventDelegate.Broadcast(Event);
}

TArray<FString> Inworld::CharactersToAgentIds(const TArray<UInworldCharacter*>& InworldCharacters)
{
	TArray<FString> AgentIds = {};
	AgentIds.Reserve(InworldCharacters.Num());
	for (const UInworldCharacter* Character : InworldCharacters)
	{
		AgentIds.Add(Character->GetAgentInfo().AgentId);
	}
	return AgentIds;
}

#undef EMPTY_ARG_RETURN
#undef NO_SESSION_RETURN
