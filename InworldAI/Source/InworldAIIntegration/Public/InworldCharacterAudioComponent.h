/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/AudioComponent.h"
#include "InworldIntegrationtypes.h"
#include "InworldCharacterMessage.h"
#include "InworldCharacterMessageQueue.h"
#include "InworldCharacterAudioComponent.generated.h"

struct FCharacterMessageUtterance;
struct FCharacterMessageSilence;

UCLASS(ClassGroup = (Inworld), meta = (BlueprintSpawnableComponent))
class INWORLDAIINTEGRATION_API UInworldCharacterAudioComponent : public UAudioComponent
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInworldCharacterVisemeBlendsUpdated, const FInworldCharacterVisemeBlends&, VisemeBlends);
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
	FOnInworldCharacterVisemeBlendsUpdated OnVisemeBlendsUpdated;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInworldCharacterBlendShapesUpdated, const FA2FBlendShapeData&, BlendShapes);
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
	FOnInworldCharacterBlendShapesUpdated OnBlendShapesUpdated;

	UFUNCTION(BlueprintPure, Category = "Sound")
	float GetAudioDuration() const;

	UFUNCTION(BlueprintPure, Category = "Sound")
	float GetAudioPlaybackPercent() const;

	UFUNCTION(BlueprintPure, Category = "Sound")
	float GetElapsedTimeForCurrentUtterance() const;

	UFUNCTION(BlueprintPure, Category = "Sound")
	float GetRemainingTimeForCurrentUtterance() const;

private:
	UFUNCTION()
	void OnCharacterUtterance(const FCharacterMessageUtterance& Message);
	UFUNCTION()
	void OnCharacterUtteranceInterrupt(const FCharacterMessageUtterance& Message);

	UFUNCTION()
	void OnCharacterUtterancePause(const FCharacterMessageUtterance& Message);
	UFUNCTION()
	void OnCharacterUtteranceResume(const FCharacterMessageUtterance& Message);

	UFUNCTION()
	void OnCharacterSilence(const FCharacterMessageSilence& Message);
	UFUNCTION()
	void OnCharacterSilenceInterrupt(const FCharacterMessageSilence& Message);
	UFUNCTION()
	void OnSilenceEnd();

	void GenerateData(class USoundWaveProcedural* InProceduralWave, int32 SamplesRequired);

	void UpdateInworldVisemeBlends();
	void UpdateA2FBlendShapes();

	void OnAudioFinished();

	TWeakObjectPtr<class UInworldCharacterComponent> CharacterComponent;
	FInworldCharacterMessageQueueLockHandle CharacterMessageQueueLockHandle;

protected:
	mutable FCriticalSection QueueLock;
	TSharedPtr<FCharacterMessageUtteranceDataAudio> UtteranceData;
	int32 NumSoundDataBytesPlayed;
	class USoundWaveProcedural* SoundStreaming;

	FTimerHandle SilenceTimerHandle;
};
