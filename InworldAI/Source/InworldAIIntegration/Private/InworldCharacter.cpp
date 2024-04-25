/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */


#include "InworldCharacter.h"
#include "InworldSession.h"
#include "InworldPlayer.h"

#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/NetDriver.h"
#include "Engine/Engine.h"

#include "Net/UnrealNetwork.h"

void UInworldCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	if (UBlueprintGeneratedClass* BPCClass = Cast<UBlueprintGeneratedClass>(GetClass()))
	{
		BPCClass->GetLifetimeBlueprintReplicationList(OutLifetimeProps);
	}

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


void UInworldCharacter::SendTextMessage(const FString& Text)
{
	UInworldSession* InworldSession = IInworldCharacterOwnerInterface::Execute_GetInworldSession(GetInworldCharacterOwner().GetObject());
	InworldSession->SendTextMessage(this, Text);
}

void UInworldCharacter::SendTrigger(const FString& Name, const TMap<FString, FString>& Params)
{
	UInworldSession* InworldSession = IInworldCharacterOwnerInterface::Execute_GetInworldSession(GetInworldCharacterOwner().GetObject());
	InworldSession->SendTrigger(this, Name, Params);
}

void UInworldCharacter::SendNarrationEvent(const FString& Content)
{
	UInworldSession* InworldSession = IInworldCharacterOwnerInterface::Execute_GetInworldSession(GetInworldCharacterOwner().GetObject());
	InworldSession->SendNarrationEvent(this, Content);
}

void UInworldCharacter::SendAudioSessionStart()
{
	UInworldSession* InworldSession = IInworldCharacterOwnerInterface::Execute_GetInworldSession(GetInworldCharacterOwner().GetObject());
	InworldSession->SendAudioSessionStart(this);
}

void UInworldCharacter::SendAudioSessionStop()
{
	UInworldSession* InworldSession = IInworldCharacterOwnerInterface::Execute_GetInworldSession(GetInworldCharacterOwner().GetObject());
	InworldSession->SendAudioSessionStop(this);
}

void UInworldCharacter::SendSoundMessage(const TArray<uint8>& InputData, const TArray<uint8>& OutputData)
{
	UInworldSession* InworldSession = IInworldCharacterOwnerInterface::Execute_GetInworldSession(GetInworldCharacterOwner().GetObject());
	InworldSession->SendSoundMessage(this, InputData, OutputData);
}

void UInworldCharacter::CancelResponse(const FString& InteractionId, const TArray<FString>& UtteranceIds)
{
	UInworldSession* InworldSession = IInworldCharacterOwnerInterface::Execute_GetInworldSession(GetInworldCharacterOwner().GetObject());
	InworldSession->CancelResponse(this, InteractionId, UtteranceIds);
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
	UInworldSession* Session = IInworldCharacterOwnerInterface::Execute_GetInworldSession(GetInworldCharacterOwner().GetObject());
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
		AgentInfo = {};
	}
}

void UInworldCharacter::SetTargetPlayer(UInworldPlayer* Player)
{
	if (Player == nullptr)
	{
		ClearTargetPlayer();
		return;
	}
	if (Player != TargetPlayer)
	{
		ClearTargetPlayer();
		TargetPlayer = Player;
		OnRep_TargetPlayer();
	}
}

void UInworldCharacter::ClearTargetPlayer()
{
	if (TargetPlayer != nullptr)
	{
		TargetPlayer = nullptr;
		OnRep_TargetPlayer();
	}
}

void UInworldCharacter::OnRep_TargetPlayer()
{
	OnTargetPlayerChangedDelegateNative.Broadcast();
	OnTargetPlayerChangedDelegate.Broadcast();
}
