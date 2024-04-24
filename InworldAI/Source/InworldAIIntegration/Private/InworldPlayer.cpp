/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */


#include "InworldPlayer.h"
#include "InworldSession.h"

TScriptInterface<IInworldPlayerOwnerInterface> UInworldPlayer::GetInworldPlayerOwner()
{
	if (!ensureMsgf(GetOuter()->Implements<UInworldPlayerOwnerInterface>(), TEXT("UInworldPlayer outer must implement IInworldPlayerOwnerInterface!")))
	{
		return nullptr;
	}
	return TScriptInterface<IInworldPlayerOwnerInterface>(GetOuter());
}
/*
void UInworldPlayer::SendTextMessageToTargets(const FString& Message)
{
	GetInworldPlayerOwner()->GetInworldSession()->BroadcastTextMessage(GetTargetInworldCharacters(), Message);
}*/