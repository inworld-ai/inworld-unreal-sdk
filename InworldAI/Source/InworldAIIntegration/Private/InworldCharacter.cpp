/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */


#include "InworldCharacter.h"
#include "InworldSession.h"

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
		Unpossess();
		AgentInfo = InAgentInfo;
		OnPossessedDelegateNative.Broadcast(true);
		OnPossessedDelegate.Broadcast(true);
	}
}

void UInworldCharacter::Unpossess()
{
	if (IsPossessed())
	{
		AgentInfo = {};
		OnPossessedDelegateNative.Broadcast(false);
		OnPossessedDelegate.Broadcast(false);
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
		OnTargetPlayerChangedDelegateNative.Broadcast();
		OnTargetPlayerChangedDelegate.Broadcast();
	}
}

void UInworldCharacter::ClearTargetPlayer()
{
	if (TargetPlayer != nullptr)
	{
		TargetPlayer = nullptr;
		OnTargetPlayerChangedDelegateNative.Broadcast();
		OnTargetPlayerChangedDelegate.Broadcast();
	}
}
