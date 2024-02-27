// Copyright 2023 Theai, Inc. (DBA Inworld) All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InworldCharacterPlayback.h"
#include "InworldCharacterPlaybackA2F.generated.h"

USTRUCT(BlueprintType)
struct FA2FBlendShapeData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	TMap<FName, float> Map;
};

UENUM(BlueprintType)
enum class EA2FAudioFormat : uint8
{
	AUDIO_FORMAT_PCM = 0,
};

USTRUCT(BlueprintType)
struct FA2FAudioHeaderData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	EA2FAudioFormat AudioFormat = EA2FAudioFormat::AUDIO_FORMAT_PCM;

	UPROPERTY(BlueprintReadOnly)
	int32 ChannelCount;

	UPROPERTY(BlueprintReadOnly)
	int32 SamplesPerSecond;

	UPROPERTY(BlueprintReadOnly)
	int32 BitsPerSample;
};

USTRUCT(BlueprintType)
struct FA2FSkeletalHeaderData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TArray<FString> BlendShapes;
};

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

	FA2FAudioHeaderData AudioHeaderData;
	FA2FSkeletalHeaderData SkeletalHeaderData;

	void OnA2FAnimationHeaderData(const FInworldA2FAnimationHeaderEvent& AnimationHeaderData);
	void OnA2FAnimationData(const FInworldA2FAnimationEvent& AnimationHeaderData);

private:
	void GenerateData(USoundWaveProcedural* InProceduralWave, int32 SamplesRequired);
	
	mutable FCriticalSection QueueLock;
	TArray<FName> BlendShapes;
	TQueue<TArray<uint8>> AudioToPlay;
	TQueue<TArray<float>> AnimsToPlay;
	int32 NumGotForUtterance = 0;
};
