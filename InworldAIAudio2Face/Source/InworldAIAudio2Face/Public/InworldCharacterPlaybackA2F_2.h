/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "InworldCharacterPlayback.h"
#include "InworldIntegrationTypes.h"
#include "InworldA2FTypes.h"
#include <Components/AudioComponent.h>
#include "InworldCharacterPlaybackA2F_2.generated.h"


/**
 * 
 */
UCLASS()
class INWORLDAIAUDIO2FACE_API UInworldCharacterPlaybackA2F_2 : public UInworldCharacterPlayback
{
	GENERATED_BODY()

public:
	virtual void BeginPlay_Implementation() override;
	virtual void EndPlay_Implementation() override;
	virtual void Tick_Implementation(float DeltaTime) override;

	virtual void OnCharacterUtterance_Implementation(const FCharacterMessageUtterance& Message) override;
	virtual void OnCharacterUtteranceInterrupt_Implementation(const FCharacterMessageUtterance& Message) override;


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
	TWeakObjectPtr<UAudioComponent> AudioComponent;
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
};
