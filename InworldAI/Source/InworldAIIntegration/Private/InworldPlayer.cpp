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

#include "InworldAIIntegrationModule.h"

#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/NetDriver.h"
#include "Engine/Engine.h"

#include "Net/UnrealNetwork.h"

#define EMPTY_ARG_RETURN(Arg, Return) INWORLD_WARN_AND_RETURN_EMPTY(LogInworldAIIntegration, UInworldPlayer, Arg, Return)
#define NO_SESSION_RETURN(Return) EMPTY_ARG_RETURN(Session, Return)

void UInworldPlayer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	if (UBlueprintGeneratedClass* BPCClass = Cast<UBlueprintGeneratedClass>(GetClass()))
	{
		BPCClass->GetLifetimeBlueprintReplicationList(OutLifetimeProps);
	}

	DOREPLIFETIME(UInworldPlayer, Session);
	DOREPLIFETIME(UInworldPlayer, TargetCharacters);
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

void UInworldPlayer::SetSession(UInworldSession* InSession)
{
	if (Session == InSession)
	{
		return;
	}

	if (Session)
	{
		Session->UnregisterPlayer(this);
	}

	Session = InSession;

	if (Session)
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

void UInworldPlayer::SendAudioSessionStartToConversation()
{
	NO_SESSION_RETURN(void())
	EMPTY_ARG_RETURN(ConversationId, void())

	if (bHasAudioSession)
	{
		return;
	}
	bHasAudioSession = true;
	Session->SendAudioSessionStartToConversation(this);
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
	Session->SendAudioSessionStopToConversation(this);
}

void UInworldPlayer::SendSoundMessageToConversation(const TArray<uint8>& Input, const TArray<uint8>& Output)
{
	NO_SESSION_RETURN(void())
	EMPTY_ARG_RETURN(ConversationId, void())
	EMPTY_ARG_RETURN(Input, void())

	if (!bHasAudioSession)
	{
		return;
	}
	Session->SendSoundMessageToConversation(this, Input, Output);
}

TScriptInterface<IInworldPlayerOwnerInterface> UInworldPlayer::GetInworldPlayerOwner()
{
	if (!ensureMsgf(GetOuter()->Implements<UInworldPlayerOwnerInterface>(), TEXT("UInworldPlayer outer must implement IInworldPlayerOwnerInterface!")))
	{
		return nullptr;
	}
	return TScriptInterface<IInworldPlayerOwnerInterface>(GetOuter());
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

void UInworldPlayer::UpdateConversation()
{
	NO_SESSION_RETURN(void())

	FString NextConversationId = Session->GetClient()->UpdateConversation(ConversationId, Inworld::CharactersToAgentIds(TargetCharacters), true);
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
		SendAudioSessionStartToConversation();
	}
}

#undef EMPTY_ARG_RETURN
#undef NO_SESSION_RETURN
