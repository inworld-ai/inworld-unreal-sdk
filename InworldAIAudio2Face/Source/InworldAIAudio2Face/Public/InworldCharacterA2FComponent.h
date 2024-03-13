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
#include "InworldA2FTypes.h"
#include "InworldCharacterMessage.h"
#include "InworldCharacterA2FComponent.generated.h"

struct FCharacterMessageUtterance;
struct FCharacterMessageSilence;

UCLASS(ClassGroup = (Inworld), meta = (BlueprintSpawnableComponent))
class INWORLDAIAUDIO2FACE_API UInworldCharacterA2FComponent : public UAudioComponent
{
	GENERATED_BODY()

public:
	UInworldCharacterA2FComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type Reason) override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInworldAudio2FaceBlendShapeUpdate, const FA2FBlendShapeData&, Blendshapes);
	UPROPERTY(BlueprintAssignable, Category = "Audio2Face")
	FOnInworldAudio2FaceBlendShapeUpdate OnInworldAudio2FaceBlendShapeUpdate;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInworldAudio2FaceBlendShapeBackupUpdate, const FInworldCharacterVisemeBlends&, VisemeBlends);
	UPROPERTY(BlueprintAssignable, Category = "Audio2Face")
	FOnInworldAudio2FaceBlendShapeBackupUpdate OnInworldAudio2FaceBlendShapeBackupUpdate;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInworldCharacterUtteranceStarted);
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
	FOnInworldCharacterUtteranceStarted OnUtteranceStarted;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInworldCharacterUtteranceStopped);
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
	FOnInworldCharacterUtteranceStopped OnUtteranceStopped;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInworldCharacterUtteranceInterrupted);
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
	FOnInworldCharacterUtteranceInterrupted OnUtteranceInterrupted;

protected:
	TWeakObjectPtr<class UInworldCharacterComponent> CharacterComponent;
	FInworldCharacterMessageQueueLockHandle CharacterMessageQueueLockHandle;
	class USoundWaveProcedural* SoundStreaming;

	void OnCharacterMessageUtteranceA2FDataUpdate();

private:
	void GenerateData(USoundWaveProcedural* InProceduralWave, int32 SamplesRequired);

	mutable FCriticalSection QueueLock;
	TQueue<TArray<uint8>> AudioToPlay;
	TQueue<TMap<FName, float>> AnimsToPlay;
	TQueue<TArray<uint8>> BackupAudioToPlay;
	TQueue<FInworldCharacterVisemeBlends> BackupAnimsToPlay;

	TArray<uint8> OriginalPCMData;
	TArray<FCharacterUtteranceVisemeInfo> VisemeInfoPlayback;

	bool bUseFallback = false;
	float AllowedLatencyDelay = 1.5f;
	float SoundDuration = 0.f;
	float SoundSize = 0.f;
	float TimeToGiveUp = 0.f;
	int32 ExpectedRemainingAudio = 0;
	int32 GotPackets = 0;
	int32 MinPacketsToStart = 20;
	bool bHasStartedProcessingAudio = false;
	bool bIsActive = false;

	TSharedPtr<FCharacterMessageUtteranceA2FData> A2FData;
	FDelegateHandle A2FDataUpdateHandle;

private:
	UFUNCTION()
	void OnCharacterUtterance(const FCharacterMessageUtterance& Message);
	UFUNCTION()
	void OnCharacterUtteranceInterrupt(const FCharacterMessageUtterance& Message);
};
