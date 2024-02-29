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

	GetCharacterComponent()->Global.A2FData->OnA2FOldAnimationHeaderData.AddUObject(this, &UInworldCharacterPlaybackA2F::OnA2FOldAnimationHeaderData);
	GetCharacterComponent()->Global.A2FData->OnA2FOldAnimationContentData.AddUObject(this, &UInworldCharacterPlaybackA2F::OnA2FOldAnimationContentData);

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
	Message.A2FData->OnA2FOldAnimationHeaderData.AddUObject(this, &UInworldCharacterPlaybackA2F::OnA2FOldAnimationHeaderData);
	Message.A2FData->OnA2FOldAnimationContentData.AddUObject(this, &UInworldCharacterPlaybackA2F::OnA2FOldAnimationContentData);

	//if (GEngine)
		//GEngine->AddOnScreenDebugMessage(67321, 15.0f, FColor::Red, FString::Printf(TEXT("I: %s, U: %s"), *Message.InteractionId, *Message.UtteranceId));
	/*
	int32 NumAudio = 0;
	int32 NumAnim = 0;

	bWaitForA2F = false;
	SkippedA2FAudio = 0;

	if (Message.SoundData.Num() > 0 && Message.bAudioFinal)
	{
		FWaveModInfo WaveInfo;
		if (!WaveInfo.ReadWaveInfo(Message.SoundData.GetData(), Message.SoundData.Num()))
		{
			return;
		}

		const float Duration = *WaveInfo.pWaveDataSize / (*WaveInfo.pChannels * (*WaveInfo.pBitsPerSample / 8.f) * *WaveInfo.pSamplesPerSec);

		TArray<int16> Data = { (int16*)WaveInfo.SampleDataStart, (int32)WaveInfo.SampleDataSize / 2 };
		Audio::AlignedFloatBuffer InputBuffer;
		InputBuffer.SetNumUninitialized(Data.Num());
		for (int32 i = 0; i < Data.Num(); ++i)
		{
			InputBuffer[i] = ((float)Data[i]) / 32767.f; // 2^15, int16
		}

		Audio::FResamplingParameters ResamplerParams = {
			Audio::EResamplingMethod::Linear,
			1,
			(float)*WaveInfo.pSamplesPerSec,
			16000.f,
			InputBuffer
		};

		Audio::FAlignedFloatBuffer OutputBuffer;
		OutputBuffer.AddUninitialized(Audio::GetOutputBufferSize(ResamplerParams));

		Audio::FResamplerResults ResamplerResults;
		ResamplerResults.OutBuffer = &OutputBuffer;

		TArray<float> ResampledAudioData;
		if (Audio::Resample(ResamplerParams, ResamplerResults))
		{
			ResampledAudioData = { ResamplerResults.OutBuffer->GetData(), ResamplerResults.OutputFramesGenerated };

			TArray<int16> PCMData;
			PCMData.AddUninitialized(ResampledAudioData.Num());
			for (int32 i = 0; i < ResampledAudioData.Num(); ++i)
			{
				PCMData[i] = ResampledAudioData[i] * 32767;  // 2^15, int16
			}

			const int32 Frames = Duration * 30.f; // 30 fps
			const int32 DataPerFrame = 533;
			for (int32 CurrentFrameStart = 0; CurrentFrameStart < PCMData.Num(); CurrentFrameStart += DataPerFrame)
			{
				const int32 CurrentFrameEnd = FMath::Clamp(CurrentFrameStart + DataPerFrame, 0, PCMData.Num() - 1);

				TArray<uint8> AudioData{ (uint8*)(PCMData.GetData() + CurrentFrameStart), (CurrentFrameEnd - CurrentFrameStart) * 2 };
				BackupAudioToPlay.Enqueue(AudioData);
				NumAudio++;
			}

			TArray<FCharacterUtteranceVisemeInfo> VisemeInfoPlayback;
			VisemeInfoPlayback.Reserve(Message.VisemeInfos.Num());

			FCharacterUtteranceVisemeInfo CurrentVisemeInfo;
			FCharacterUtteranceVisemeInfo PreviousVisemeInfo;

			for (const auto& VisemeInfo : Message.VisemeInfos)
			{
				if (!VisemeInfo.Code.IsEmpty())
				{
					VisemeInfoPlayback.Add(VisemeInfo);
				}
			}

			for(float CurrentAudioPlaybackTime = 0.f; CurrentAudioPlaybackTime < Duration; CurrentAudioPlaybackTime += 1.f/30.f)
			{
				const int32 INVALID_INDEX = -1;
				int32 Target = INVALID_INDEX;
				int32 L = 0;
				int32 R = VisemeInfoPlayback.Num() - 1;
				while (L <= R)
				{
					const int32 Mid = (L + R) >> 1;
					const FCharacterUtteranceVisemeInfo& Sample = VisemeInfoPlayback[Mid];
					if (CurrentAudioPlaybackTime > Sample.Timestamp)
					{
						L = Mid + 1;
					}
					else
					{
						Target = Mid;
						R = Mid - 1;
					}
				}
				if (VisemeInfoPlayback.IsValidIndex(Target))
				{
					CurrentVisemeInfo = VisemeInfoPlayback[Target];
				}
				if (VisemeInfoPlayback.IsValidIndex(Target - 1))
				{
					PreviousVisemeInfo = VisemeInfoPlayback[Target - 1];
				}

				const float Blend = (CurrentAudioPlaybackTime - PreviousVisemeInfo.Timestamp) / (CurrentVisemeInfo.Timestamp - PreviousVisemeInfo.Timestamp);

				FInworldCharacterVisemeBlends VisemeBlends;
				VisemeBlends[PreviousVisemeInfo.Code] = FMath::Clamp(1.f - Blend, 0.f, 1.f);
				VisemeBlends[CurrentVisemeInfo.Code] = FMath::Clamp(Blend, 0.f, 1.f);

				BackupAnimsToPlay.Enqueue(VisemeBlends);
				NumAnim++;
			}
		}
	}
	*/
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

void UInworldCharacterPlaybackA2F::OnA2FOldAnimationHeaderData(const FInworldA2FOldAnimationHeaderEvent& AnimationHeaderData)
{
	// Do Nothing...
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

		TMap<FName, float> AnimToPlay;
		for (int32 i = 0; i < BlendShapes.Num(); ++i)
		{
			AnimToPlay.Add(BlendShapes[i], AnimationData.BlendShapeWeights.Values[i]);
		}

		AnimsToPlay.Enqueue(AnimToPlay);
	}

	bWaitForA2F = false;
}

void UInworldCharacterPlaybackA2F::OnA2FOldAnimationContentData(const FInworldA2FOldAnimationContentEvent& AnimationData)
{
	if (AnimationData.Audio.IsEmpty() || AnimationData.BlendShapeMap.IsEmpty())
	{
		return;
	}

	//if (GEngine)
		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("I: %s, U: %s"), *AnimationData.PacketId.InteractionId, *AnimationData.PacketId.UtteranceId));

	{
		AudioToPlay.Enqueue(AnimationData.Audio);
		AnimsToPlay.Enqueue(AnimationData.BlendShapeMap);
	}

	bWaitForA2F = false;

	if (!AudioComponent->IsActive())
	{
		AudioComponent->Play();
	}
}

void UInworldCharacterPlaybackA2F::GenerateData(USoundWaveProcedural* InProceduralWave, int32 SamplesRequired)
{
	FScopeLock ScopeLock(&QueueLock);
	/*if (BackupAudioToPlay.IsEmpty())
	{
		OnUtteranceStopped.Broadcast();
		UnlockMessageQueue();
	}
	else if (AudioToPlay.IsEmpty())
	{
		SkippedA2FAudio++;
		TArray<uint8> ToQueue;
		BackupAudioToPlay.Dequeue(ToQueue);
		InProceduralWave->QueueAudio(ToQueue.GetData(), ToQueue.Num());

		FInworldCharacterVisemeBlends AnimToPlay;
		BackupAnimsToPlay.Dequeue(AnimToPlay);
		AsyncTask(ENamedThreads::GameThread, [this, AnimToPlay]()
		{
			OnInworldAudio2FaceBlendShapeBackupUpdate.Broadcast(AnimToPlay);
		});
	}
	else
	{
		BackupAudioToPlay.Pop();
		BackupAnimsToPlay.Pop();
	*/
		TArray<uint8> ToQueue;
		AudioToPlay.Dequeue(ToQueue);
		InProceduralWave->QueueAudio(ToQueue.GetData(), ToQueue.Num());
		
		TMap<FName, float> AnimToPlay;
		AnimsToPlay.Dequeue(AnimToPlay);
		AsyncTask(ENamedThreads::GameThread, [this, AnimToPlay]()
		{
			FA2FBlendShapeData Data;
			Data.Map = AnimToPlay;
			OnInworldAudio2FaceBlendShapeUpdate.Broadcast(Data);
			if (AudioToPlay.IsEmpty())
			{
				//OnUtteranceStopped.Broadcast();
				//UnlockMessageQueue();
			}
		});
	//}
}