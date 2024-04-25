/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */


#include "InworldPlayer.h"
#include "InworldCharacter.h"
#include "InworldSession.h"

#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/NetDriver.h"
#include "Engine/Engine.h"

#include "Net/UnrealNetwork.h"

void UInworldPlayer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	if (UBlueprintGeneratedClass* BPCClass = Cast<UBlueprintGeneratedClass>(GetClass()))
	{
		BPCClass->GetLifetimeBlueprintReplicationList(OutLifetimeProps);
	}

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

void UInworldPlayer::BroadcastTextMessage(const FString& Text)
{
	UInworldSession* InworldSession = IInworldCharacterOwnerInterface::Execute_GetInworldSession(GetInworldPlayerOwner().GetObject());
	InworldSession->BroadcastTextMessage(GetTargetCharacters(), Text);
}

void UInworldPlayer::BroadcastTrigger(const FString& Name, const TMap<FString, FString>& Params)
{
	UInworldSession* InworldSession = IInworldCharacterOwnerInterface::Execute_GetInworldSession(GetInworldPlayerOwner().GetObject());
	InworldSession->BroadcastTrigger(GetTargetCharacters(), Name, Params);
}

void UInworldPlayer::BroadcastAudioSessionStart()
{
	UInworldSession* InworldSession = IInworldCharacterOwnerInterface::Execute_GetInworldSession(GetInworldPlayerOwner().GetObject());
	InworldSession->BroadcastAudioSessionStart(GetTargetCharacters());
}

void UInworldPlayer::BroadcastAudioSessionStop()
{
	UInworldSession* InworldSession = IInworldCharacterOwnerInterface::Execute_GetInworldSession(GetInworldPlayerOwner().GetObject());
	InworldSession->BroadcastAudioSessionStop(GetTargetCharacters());
}

void UInworldPlayer::BroadcastSoundMessage(const TArray<uint8>& Input, const TArray<uint8>& Output)
{
	UInworldSession* InworldSession = IInworldCharacterOwnerInterface::Execute_GetInworldSession(GetInworldPlayerOwner().GetObject());
	InworldSession->BroadcastSoundMessage(GetTargetCharacters(), Input, Output);
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
	if (TargetCharacter->GetTargetPlayer() == nullptr)
	{
		TargetCharacter->SetTargetPlayer(this);
		TargetCharacters.AddUnique(TargetCharacter);
		OnTargetCharacterAddedDelegateNative.Broadcast(TargetCharacter);
		OnTargetCharacterAddedDelegate.Broadcast(TargetCharacter);

		OnTargetCharactersChangedDelegateNative.Broadcast();
		OnTargetCharactersChangedDelegate.Broadcast();
	}
}

void UInworldPlayer::RemoveTargetCharacter(UInworldCharacter* TargetCharacter)
{
	if (TargetCharacter->GetTargetPlayer() == this)
	{
		TargetCharacter->ClearTargetPlayer();
		TargetCharacters.RemoveSingle(TargetCharacter);
		OnTargetCharacterRemovedDelegateNative.Broadcast(TargetCharacter);
		OnTargetCharacterRemovedDelegate.Broadcast(TargetCharacter);

		OnTargetCharactersChangedDelegateNative.Broadcast();
		OnTargetCharactersChangedDelegate.Broadcast();
	}
}

void UInworldPlayer::ClearAllTargetCharacters()
{
	TArray<UInworldCharacter*> TargetCharactersCopy = TargetCharacters;
	for (UInworldCharacter* TargetCharacter : TargetCharactersCopy)
	{
		bool bRemovedAny = false;
		if (TargetCharacter->GetTargetPlayer() == this)
		{
			TargetCharacter->ClearTargetPlayer();
			TargetCharacters.RemoveSingle(TargetCharacter);
			OnTargetCharacterRemovedDelegateNative.Broadcast(TargetCharacter);
			OnTargetCharacterRemovedDelegate.Broadcast(TargetCharacter);
			bRemovedAny = true;
		}
		if (bRemovedAny)
		{
			OnTargetCharactersChangedDelegateNative.Broadcast();
			OnTargetCharactersChangedDelegate.Broadcast();
		}
	}
}
