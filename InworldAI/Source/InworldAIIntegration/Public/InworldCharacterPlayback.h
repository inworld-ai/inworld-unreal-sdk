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
	 */
	UFUNCTION(BlueprintNativeEvent)
	void Tick(float DeltaTime);

	/**
	 * Event for when Character has changed interaction states
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Playback|Interaction")
	void OnCharacterInteractionState(bool bInteracting);

	/**
	 * Event for when Character has been talked to by the player
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Playback|Interaction")
	void OnCharacterPlayerTalk(const FCharacterMessagePlayerTalk& Message);

	/**
	 * Event for when Character has changed emotions
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Playback|Emotion")
	void OnCharacterEmotion(EInworldCharacterEmotionalBehavior Emotion, EInworldCharacterEmotionStrength Strength);

	/**
	 * Event for when Character has uttered
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Playback|Utterance")
	void OnCharacterUtterance(const FCharacterMessageUtterance& Message);

	/**
	 * Event for when Character is interrupted while uttering
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Playback|Utterance")
	void OnCharacterUtteranceInterrupt(const FCharacterMessageUtterance& Message);

	/**
	 * Event for when Character is silenced
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Playback|Silence")
	void OnCharacterSilence(const FCharacterMessageSilence& Message);

	/**
	 * Event for when Character is interrupted while silenced
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Playback|Silence")
	void OnCharacterSilenceInterrupt(const FCharacterMessageSilence& Message);

	/**
	 * Event for when Character has a trigger
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Playback|Trigger")
	void OnCharacterTrigger(const FCharacterMessageTrigger& Message);

	/**
	 * Event for when Character interaction ends
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Playback|Interaction")
	void OnCharacterInteractionEnd(const FCharacterMessageInteractionEnd& Message);

	/**
	 * Locks the Character's message queue
	 */
	UFUNCTION(BlueprintCallable, Category = "Message Queue")
	bool LockMessageQueue();

	/**
	 * Unlocks the Character's message queue
	 */
	UFUNCTION(BlueprintCallable, Category = "Message Queue")
	void UnlockMessageQueue();

	void SetCharacterComponent(class UInworldCharacterComponent* InCharacterComponent);
	void ClearCharacterComponent();

protected:

	UFUNCTION(BlueprintPure, Category = "Inworld")
	const UInworldCharacterComponent* GetCharacterComponent() const
	{
		return CharacterComponent.Get();
	}

	UFUNCTION(BlueprintPure, Category = "Inworld")
	const AActor* GetOwner() const
	{
		return OwnerActor.Get();
	}

	TWeakObjectPtr<AActor> OwnerActor;
	TWeakObjectPtr<class UInworldCharacterComponent> CharacterComponent;

	FInworldCharacterMessageQueueLockHandle CharacterMessageQueueLockHandle;
};
