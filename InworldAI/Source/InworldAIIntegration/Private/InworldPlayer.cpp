/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */


#include "InworldPlayer.h"
#include "InworldCharacter.h"
#include "InworldSession.h"

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
