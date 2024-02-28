// Copyright 2023 Theai, Inc. (DBA Inworld) All Rights Reserved.


#include "InworldCharacterPlaybackA2F.h"
#include "InworldCharacterComponent.h"
#include "InworldBlueprintFunctionLibrary.h"
#include "AudioResampler.h"
#include "Sound/SoundWave.h"
#include "Sound/SoundWaveProcedural.h"
#include <Components/AudioComponent.h>


void UInworldCharacterPlaybackA2F::BeginPlay_Implementation()
{
	Super::BeginPlay_Implementation();

	AudioComponent = Cast<UAudioComponent>(OwnerActor->GetComponentByClass(UAudioComponent::StaticClass()));

	GetCharacterComponent()->Global.A2FData->OnA2FAnimationHeaderData.AddUObject(this, &UInworldCharacterPlaybackA2F::OnA2FAnimationHeaderData);
	GetCharacterComponent()->Global.A2FData->OnA2FAnimationData.AddUObject(this, &UInworldCharacterPlaybackA2F::OnA2FAnimationData);

	if (ensureMsgf(AudioComponent.IsValid(), TEXT("UInworldCharacterPlaybackA2F owner doesn't contain AudioComponent")))
	{
		SoundStreaming = NewObject<USoundWaveProcedural>();
		SoundStreaming->SetSampleRate(16000);
		SoundStreaming->NumChannels = 1;
		SoundStreaming->Duration = INDEFINITELY_LOOPING_DURATION;
		SoundStreaming->SoundGroup = SOUNDGROUP_Voice;
		SoundStreaming->bLooping = false;

		SoundStreaming->OnSoundWaveProceduralUnderflow = FOnSoundWaveProceduralUnderflow::CreateUObject(this, &UInworldCharacterPlaybackA2F::GenerateData);

		AudioComponent->SetSound(SoundStreaming);
	}
}

void UInworldCharacterPlaybackA2F::EndPlay_Implementation()
{
	Super::EndPlay_Implementation();

	SoundStreaming->ResetAudio();
	OnUtteranceInterrupted.Broadcast();
	OnUtteranceStopped.Broadcast();
	UnlockMessageQueue();
}

void UInworldCharacterPlaybackA2F::Tick_Implementation(float DeltaTime)
{
	Super::Tick_Implementation(DeltaTime);
}

void UInworldCharacterPlaybackA2F::OnCharacterUtterance_Implementation(const FCharacterMessageUtterance& Message)
{
	Message.A2FData->OnA2FAnimationHeaderData.AddUObject(this, &UInworldCharacterPlaybackA2F::OnA2FAnimationHeaderData);
	Message.A2FData->OnA2FAnimationData.AddUObject(this, &UInworldCharacterPlaybackA2F::OnA2FAnimationData);
	NumGotForUtterance = 0;
	LockMessageQueue();
}

void UInworldCharacterPlaybackA2F::OnCharacterUtteranceInterrupt_Implementation(const FCharacterMessageUtterance& Message)
{
	SoundStreaming->ResetAudio();
	OnUtteranceInterrupted.Broadcast();
	OnUtteranceStopped.Broadcast();
	UnlockMessageQueue();
}


void UInworldCharacterPlaybackA2F::OnA2FAnimationHeaderData(const FInworldA2FAnimationHeaderEvent& AnimationHeaderData)
{
	BlendShapes = AnimationHeaderData.BlendShapes;
}

void UInworldCharacterPlaybackA2F::OnA2FAnimationData(const FInworldA2FAnimationEvent& AnimationData)
{
	{
		FScopeLock ScopeLock(&QueueLock);

		TArray<float> FloatData{ (float*)(AnimationData.AudioInfo.Audio.GetData()), (AnimationData.AudioInfo.Audio.Num()) / 4 };
		TArray<int16> PCMData;
		PCMData.AddUninitialized(FloatData.Num());
		for (int32 i = 0; i < FloatData.Num(); ++i)
		{
			PCMData[i] = FloatData[i] * 32767;  // 2^15, int16
		}

		AudioToPlay.Enqueue({ (uint8*)PCMData.GetData(), PCMData.Num() * 2 });
		AnimsToPlay.Enqueue(AnimationData.BlendShapeWeights.Values);
	}

	NumGotForUtterance++;
	if (!AudioComponent->IsActive())
	{
		AudioComponent->Play();
	}
}

void UInworldCharacterPlaybackA2F::GenerateData(USoundWaveProcedural* InProceduralWave, int32 SamplesRequired)
{
	//if (NumGotForUtterance < 25) return;
	FScopeLock ScopeLock(&QueueLock);
	if (!AudioToPlay.IsEmpty())
	{
		TArray<uint8> ToQueue;
		AudioToPlay.Dequeue(ToQueue);
		InProceduralWave->QueueAudio(ToQueue.GetData(), ToQueue.Num());
		
		TArray<float> BlendShapeValues;
		AnimsToPlay.Dequeue(BlendShapeValues);
		AsyncTask(ENamedThreads::GameThread, [this, BlendShapeValues]()
		{
			FA2FBlendShapeData Data;
			for (int32 i = 0; i < BlendShapes.Num(); ++i)
			{
				Data.Map.Add(BlendShapes[i], BlendShapeValues[i]);
			}
			OnInworldAudio2FaceBlendShapeUpdate.Broadcast(Data);

			if (AudioToPlay.IsEmpty())
			{
				OnUtteranceStopped.Broadcast();
				UnlockMessageQueue();
			}
		});
		
	}
}