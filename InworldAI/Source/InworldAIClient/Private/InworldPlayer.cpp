/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */


#include "InworldPlayer.h"
#include "InworldCharacter.h"
#include "InworldSession.h"
#include "InworldMacros.h"

#include "InworldAIClientModule.h"

#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/NetDriver.h"
#include "Engine/Engine.h"

#include "Net/UnrealNetwork.h"

#define EMPTY_ARG_RETURN(Arg, Return) INWORLD_WARN_AND_RETURN_EMPTY(LogInworldAIClient, UInworldPlayer, Arg, Return)
#define NO_SESSION_RETURN(Return) EMPTY_ARG_RETURN(Session, Return)

UInworldPlayer::UInworldPlayer()
	: Super()
	, PacketVisitor(MakeShared<FInworldPlayerPacketVisitor>(this))
{}

UInworldPlayer::~UInworldPlayer()
{}

void UInworldPlayer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	if (UBlueprintGeneratedClass* BPCClass = Cast<UBlueprintGeneratedClass>(GetClass()))
	{
		BPCClass->GetLifetimeBlueprintReplicationList(OutLifetimeProps);
	}

	DOREPLIFETIME(UInworldPlayer, Session);
	DOREPLIFETIME(UInworldPlayer, bConversationParticipant);
	DOREPLIFETIME(UInworldPlayer, TargetCharacters);
	DOREPLIFETIME(UInworldPlayer, bVoiceDetected);
}

int32 UInworldPlayer::GetFunctionCallspace(UFunction* Function, FFrame* Stack)
{
	if (HasAnyFlags(RF_ClassDefaultObject) || !IsSupportedForNetworking())
	{
		return GEngine->GetGlobalFunctionCallspace(Function, this, Stack);
	}

	return GetOuter()->GetFunctionCallspace(Function, Stack);
}

bool UInworldPlayer::CallRemoteFunction(UFunction* Function, void* Parms, FOutParmRec* OutParms, FFrame* Stack)
{
	AActor* Owner = GetTypedOuter<AActor>();
	if (UNetDriver* NetDriver = Owner->GetNetDriver())
	{
		NetDriver->ProcessRemoteFunction(Owner, Function, Parms, OutParms, Stack, this);
		return true;
	}
	return false;
}

void UInworldPlayer::HandlePacket(const FInworldWrappedPacket& WrappedPacket)
{
	auto& Packet = WrappedPacket.Packet;
	if (Packet.IsValid())
	{
		Packet->Accept(*PacketVisitor);
	}
}

void UInworldPlayer::SetSession(UInworldSession* InSession)
{
	if (Session == InSession)
	{
		return;
	}

	if (Session.IsValid())
	{
		Session->UnregisterPlayer(this);
	}

	Session = InSession;

	if (Session.IsValid())
	{
		Session->RegisterPlayer(this);
	}
}

void UInworldPlayer::SendTextMessageToConversation(const FString& Text)
{
	NO_SESSION_RETURN(void())
	EMPTY_ARG_RETURN(ConversationId, void())
	EMPTY_ARG_RETURN(Text, void())

	Session->SendTextMessageToConversation(this, Text);
}

void UInworldPlayer::SendTriggerToConversation(const FString& Name, const TMap<FString, FString>& Params)
{
	NO_SESSION_RETURN(void())
	EMPTY_ARG_RETURN(ConversationId, void())
	EMPTY_ARG_RETURN(Name, void())

	Session->SendTriggerToConversation(this, Name, Params);
}

void UInworldPlayer::SendAudioSessionStartToConversation(FInworldAudioSessionOptions InAudioSessionOptions)
{
	NO_SESSION_RETURN(void())
	EMPTY_ARG_RETURN(ConversationId, void())

	if (bHasAudioSession || !bConversationParticipant)
	{
		return;
	}
	bHasAudioSession = true;

	AudioSessionOptions = InAudioSessionOptions;
	Session->SendAudioSessionStartToConversation(this, AudioSessionOptions);
}

void UInworldPlayer::SendAudioSessionStopToConversation()
{
	NO_SESSION_RETURN(void())
	EMPTY_ARG_RETURN(ConversationId, void())

	if (!bHasAudioSession)
	{
		return;
	}
	bHasAudioSession = false;

	AudioSessionOptions = {};
	Session->SendAudioSessionStopToConversation(this);
}

void UInworldPlayer::SendSoundMessageToConversation(const TArray<uint8>& Input, const TArray<uint8>& Output)
{
	NO_SESSION_RETURN(void())
	EMPTY_ARG_RETURN(ConversationId, void())
	EMPTY_ARG_RETURN(Input, void())

	if (!bHasAudioSession || !bConversationParticipant)
	{
		return;
	}
	Session->SendSoundMessageToConversation(this, Input, Output);
}

void UInworldPlayer::SetConversationParticipation(bool bParticipate)
{
	if (bConversationParticipant != bParticipate)
	{
		bConversationParticipant = bParticipate;
		if (!ConversationId.IsEmpty())
		{
			UpdateConversation();
		}
	}
}

void UInworldPlayer::AddTargetCharacter(UInworldCharacter* TargetCharacter)
{
	if (TargetCharacter && TargetCharacter->IsPossessed() && TargetCharacter->GetTargetPlayer() == nullptr)
	{
		TargetCharacter->SetTargetPlayer(this);
		TargetCharacters.AddUnique(TargetCharacter);

		UpdateConversation();

		OnTargetCharacterAddedDelegateNative.Broadcast(TargetCharacter);
		OnTargetCharacterAddedDelegate.Broadcast(TargetCharacter);

		OnTargetCharactersChangedDelegateNative.Broadcast();
		OnTargetCharactersChangedDelegate.Broadcast();
	}
}

void UInworldPlayer::RemoveTargetCharacter(UInworldCharacter* TargetCharacter)
{
	if (TargetCharacter && TargetCharacter->GetTargetPlayer() == this)
	{
		TargetCharacter->ClearTargetPlayer();
		TargetCharacters.RemoveSingle(TargetCharacter);

		UpdateConversation();

		OnTargetCharacterRemovedDelegateNative.Broadcast(TargetCharacter);
		OnTargetCharacterRemovedDelegate.Broadcast(TargetCharacter);

		OnTargetCharactersChangedDelegateNative.Broadcast();
		OnTargetCharactersChangedDelegate.Broadcast();
	}
}

void UInworldPlayer::ClearAllTargetCharacters()
{
	TArray<UInworldCharacter*> CharactersToRemove = {};
	for (UInworldCharacter* TargetCharacter : TargetCharacters)
	{
		if (TargetCharacter->GetTargetPlayer() == this)
		{
			CharactersToRemove.Add(TargetCharacter);
		}
	}
	if (CharactersToRemove.Num() > 0)
	{
		for (UInworldCharacter* CharacterToRemove : CharactersToRemove)
		{
			CharacterToRemove->ClearTargetPlayer();
			TargetCharacters.RemoveSingle(CharacterToRemove);
		}

		UpdateConversation();

		for (UInworldCharacter* CharacterToRemove : CharactersToRemove)
		{
			OnTargetCharacterRemovedDelegateNative.Broadcast(CharacterToRemove);
			OnTargetCharacterRemovedDelegate.Broadcast(CharacterToRemove);
		}

		OnTargetCharactersChangedDelegateNative.Broadcast();
		OnTargetCharactersChangedDelegate.Broadcast();
	}
}

void UInworldPlayer::OnRep_VoiceDetected(bool bOldValue)
{
	if (bVoiceDetected != bOldValue)
	{
		OnVoiceDetectionDelegate.Broadcast(bVoiceDetected);
		OnVoiceDetectionDelegateNative.Broadcast(bVoiceDetected);
	}
}

void UInworldPlayer::UpdateConversation()
{
	NO_SESSION_RETURN(void())

	FString NextConversationId = Session->UpdateConversation(this);
	const bool bHadAudioSession = bHasAudioSession;
	if (bHasAudioSession)
	{
		SendAudioSessionStopToConversation();
	}

	ConversationId = NextConversationId;
	OnConversationChangedDelegateNative.Broadcast();
	OnConversationChangedDelegate.Broadcast();

	if (bHadAudioSession && !ConversationId.IsEmpty())
	{
		SendAudioSessionStartToConversation(AudioSessionOptions);
	}
}

void UInworldPlayer::FInworldPlayerPacketVisitor::Visit(const FInworldVADEvent& Event)
{
	const bool bOldValue = Player->bVoiceDetected;
	Player->bVoiceDetected = Event.VoiceDetected;
	Player->OnRep_VoiceDetected(bOldValue);
}

void UInworldPlayer::FInworldPlayerPacketVisitor::Visit(const FInworldConversationUpdateEvent& Event)
{
	if (Event.EventType == EInworldConversationUpdateType::EVICTED)
	{
		Player->ConversationId = {};
		Player->UpdateConversation();
	}
}

#undef EMPTY_ARG_RETURN
#undef NO_SESSION_RETURN
