// Copyright 2023 Theai, Inc. (DBA Inworld) All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InworldCharacterPlayback.h"
#include "InworldAudio2Face.h"
#include "InworldA2FTypes.h"
#include "InworldCharacterPlaybackA2F.generated.h"

/**
 *
 */
UCLASS()
class INWORLDAIAUDIO2FACE_API UInworldCharacterPlaybackA2F : public UInworldCharacterPlayback
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

	//DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInworldCharacterUtteranceStarted);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInworldCharacterUtteranceStarted, const FCharacterMessageUtterance&, Utterance, const AActor*, Owner);
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

private:
	void OnInworldAudio2FaceAnimDataHeader(const FAudio2FaceAnimDataHeader& Header);
	void OnInworldAudio2FaceAnimDataContent(const FAudio2FaceAnimDataContent& Content);

	void GenerateData(USoundWaveProcedural* InProceduralWave, int32 SamplesRequired);

private:
	FInworldAudio2Face InworldAudio2Face;
	TQueue<TArray<uint8>> AudioToSend;

	mutable FCriticalSection QueueLock;
	TQueue<TArray<uint8>> AudioToPlay;
	TQueue<TMap<FName, float>> AnimsToPlay;
	int32 RemainingAudio = 0;
	bool bIsActive = false;
	int32 GotPackets = 0;
	int32 MinPacketsToStart = 6;
};