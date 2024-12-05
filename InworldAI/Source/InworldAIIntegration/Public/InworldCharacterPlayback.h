/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "InworldCharacterMessage.h"
#include "InworldCharacterMessageQueue.h"
#include "InworldEnums.h"

#include "InworldCharacterPlayback.generated.h"

class UInworldCharacterComponent;

UCLASS(BlueprintType, Blueprintable, Abstract)
class INWORLDAIINTEGRATION_API UInworldCharacterPlayback : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Use for initializing
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Playback")
	void BeginPlay();

	/**
	 * Use for deinitializing
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Playback")
	void EndPlay();

	/**
	 * Use for updating
	 * @param DeltaTime The time since the last tick.
	 */
	UFUNCTION(BlueprintNativeEvent)
	void Tick(float DeltaTime);

	/**
	 * Event for when Character has changed interaction states
	 * @param bInteracting Indicates if the character is currently interacting.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Playback|Interaction")
	void OnCharacterInteractionState(bool bInteracting);

	/**
	 * Event for when Character has been talked to by the player
	 * @param Message The message sent by the player.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Playback|Interaction")
	void OnCharacterPlayerTalk(const FCharacterMessagePlayerTalk& Message);

	/**
	 * Event for when Character has changed emotions
	 * @param Emotion The emotional behavior of the character.
	 * @param Strength The strength of the emotion.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Playback|Emotion")
	void OnCharacterEmotion(EInworldCharacterEmotionalBehavior Emotion, EInworldCharacterEmotionStrength Strength);

	/**
	 * Event for when Character has uttered
	 * @param Message The utterance message.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Playback|Utterance")
	void OnCharacterUtterance(const FCharacterMessageUtterance& Message);

	/**
	 * Event for when Character is interrupted while uttering
	 * @param Message The utterance message being interrupted.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Playback|Utterance")
	void OnCharacterUtteranceInterrupt(const FCharacterMessageUtterance& Message);

	/**
	 * Event for when Character is silenced
	 * @param Message The silence message.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Playback|Silence")
	void OnCharacterSilence(const FCharacterMessageSilence& Message);

	/**
	 * Event for when Character is interrupted while silenced
	 * @param Message The silence message being interrupted.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Playback|Silence")
	void OnCharacterSilenceInterrupt(const FCharacterMessageSilence& Message);

	/**
	 * Event for when Character has a trigger
	 * @param Message The trigger message.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Playback|Trigger")
	void OnCharacterTrigger(const FCharacterMessageTrigger& Message);

	/**
	 * Event for when Character interaction ends
	 * @param Message The interaction end message.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Playback|Interaction")
	void OnCharacterInteractionEnd(const FCharacterMessageInteractionEnd& Message);

	/**
	 * Locks the Character's message queue
	 */
	UFUNCTION(BlueprintCallable, Category = "Message Queue")
	void LockMessageQueue();

	/**
	 * Unlocks the Character's message queue
	 */
	UFUNCTION(BlueprintCallable, Category = "Message Queue")
	void UnlockMessageQueue();

	void SetCharacterComponent(class UInworldCharacterComponent* InCharacterComponent);
	void ClearCharacterComponent();

protected:

	/**
	 * Get the character component associated with this actor.
	 * @return The character component.
	 */
	UFUNCTION(BlueprintPure, Category = "Inworld")
	const UInworldCharacterComponent* GetCharacterComponent() const
	{
		return CharacterComponent.Get();
	}

	/**
	 * Get the owner actor of this component.
	 * @return The owner actor.
	 */
	UFUNCTION(BlueprintPure, Category = "Inworld")
	const AActor* GetOwner() const
	{
		return OwnerActor.Get();
	}

	TWeakObjectPtr<AActor> OwnerActor;
	TWeakObjectPtr<UInworldCharacterComponent> CharacterComponent;

	FInworldCharacterMessageQueueLockHandle CharacterMessageQueueLockHandle;
};
