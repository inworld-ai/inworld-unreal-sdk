/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldCharacterPlayback.h"
#include "InworldCharacterComponent.h"
#include <GameFramework/Actor.h>

void UInworldCharacterPlayback::BeginPlay_Implementation() {}
void UInworldCharacterPlayback::EndPlay_Implementation() {}
void UInworldCharacterPlayback::Tick_Implementation(float DeltaTime) {}
void UInworldCharacterPlayback::OnCharacterInteractionState_Implementation(bool bInteracting) {}
void UInworldCharacterPlayback::OnCharacterPlayerTalk_Implementation(const FCharacterMessagePlayerTalk& Message) {}
void UInworldCharacterPlayback::OnCharacterEmotion_Implementation(EInworldCharacterEmotionalBehavior Emotion, EInworldCharacterEmotionStrength Strength) {}
void UInworldCharacterPlayback::OnCharacterUtterance_Implementation(const FCharacterMessageUtterance& Message) {}
void UInworldCharacterPlayback::OnCharacterUtteranceInterrupt_Implementation(const FCharacterMessageUtterance& Message) {}
void UInworldCharacterPlayback::OnCharacterSilence_Implementation(const FCharacterMessageSilence& Message) {}
void UInworldCharacterPlayback::OnCharacterSilenceInterrupt_Implementation(const FCharacterMessageSilence& Message) {}
void UInworldCharacterPlayback::OnCharacterTrigger_Implementation(const FCharacterMessageTrigger& Message) {}
void UInworldCharacterPlayback::OnCharacterInteractionEnd_Implementation(const FCharacterMessageInteractionEnd& Message) {}

void UInworldCharacterPlayback::SetCharacterComponent(UInworldCharacterComponent* InCharacterComponent)
{
	if (!ensure(InCharacterComponent))
	{
		return;
	}
	CharacterComponent = InCharacterComponent;
	OwnerActor = CharacterComponent->GetOwner();

	CharacterComponent->OnPlayerInteractionStateChanged.AddDynamic(this, &UInworldCharacterPlayback::OnCharacterInteractionState);
	CharacterComponent->OnPlayerTalk.AddDynamic(this, &UInworldCharacterPlayback::OnCharacterPlayerTalk);

	CharacterComponent->OnEmotionalBehaviorChanged.AddDynamic(this, &UInworldCharacterPlayback::OnCharacterEmotion);

	CharacterComponent->OnUtterance.AddDynamic(this, &UInworldCharacterPlayback::OnCharacterUtterance);
	CharacterComponent->OnUtteranceInterrupt.AddDynamic(this, &UInworldCharacterPlayback::OnCharacterUtteranceInterrupt);

	CharacterComponent->OnSilence.AddDynamic(this, &UInworldCharacterPlayback::OnCharacterSilence);
	CharacterComponent->OnSilenceInterrupt.AddDynamic(this, &UInworldCharacterPlayback::OnCharacterSilenceInterrupt);

	CharacterComponent->OnTrigger.AddDynamic(this, &UInworldCharacterPlayback::OnCharacterTrigger);
	CharacterComponent->OnInteractionEnd.AddDynamic(this, &UInworldCharacterPlayback::OnCharacterInteractionEnd);
}

void UInworldCharacterPlayback::ClearCharacterComponent()
{
	if (!ensure(CharacterComponent.IsValid()))
	{
		return;
	}

	CharacterComponent->OnPlayerInteractionStateChanged.RemoveDynamic(this, &UInworldCharacterPlayback::OnCharacterInteractionState);
	CharacterComponent->OnPlayerTalk.RemoveDynamic(this, &UInworldCharacterPlayback::OnCharacterPlayerTalk);

	CharacterComponent->OnEmotionalBehaviorChanged.RemoveDynamic(this, &UInworldCharacterPlayback::OnCharacterEmotion);

	CharacterComponent->OnUtterance.RemoveDynamic(this, &UInworldCharacterPlayback::OnCharacterUtterance);
	CharacterComponent->OnUtteranceInterrupt.RemoveDynamic(this, &UInworldCharacterPlayback::OnCharacterUtteranceInterrupt);

	CharacterComponent->OnSilence.RemoveDynamic(this, &UInworldCharacterPlayback::OnCharacterSilence);
	CharacterComponent->OnSilenceInterrupt.RemoveDynamic(this, &UInworldCharacterPlayback::OnCharacterSilenceInterrupt);

	CharacterComponent->OnTrigger.RemoveDynamic(this, &UInworldCharacterPlayback::OnCharacterTrigger);
	CharacterComponent->OnInteractionEnd.RemoveDynamic(this, &UInworldCharacterPlayback::OnCharacterInteractionEnd);

	CharacterComponent = nullptr;
	OwnerActor = nullptr;
}

void UInworldCharacterPlayback::LockMessageQueue()
{
	CharacterComponent->MakeMessageQueueLock(CharacterMessageQueueLockHandle);
}

void UInworldCharacterPlayback::UnlockMessageQueue()
{
	CharacterComponent->ClearMessageQueueLock(CharacterMessageQueueLockHandle);
}
