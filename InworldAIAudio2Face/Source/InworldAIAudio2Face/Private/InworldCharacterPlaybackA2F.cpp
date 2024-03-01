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
	TimeToGiveUp -= DeltaTime;
	if (!AudioComponent->IsActive())
	{
		AudioComponent->Play();
	}
}

void UInworldCharacterPlaybackA2F::OnCharacterUtterance_Implementation(const FCharacterMessageUtterance& Message)
{

	if (Message.SoundData.Num() > 0 && Message.bAudioFinal)
	{
		A2FData = Message.A2FData;
		HeaderDataHandle = A2FData->OnA2FOldAnimationHeaderData.AddUObject(this, &UInworldCharacterPlaybackA2F::OnA2FOldAnimationHeaderData);
		ContentDataHandle = A2FData->OnA2FOldAnimationContentData.AddUObject(this, &UInworldCharacterPlaybackA2F::OnA2FOldAnimationContentData);

		FScopeLock ScopeLock(&QueueLock);
		AudioToPlay.Empty();
		AnimsToPlay.Empty();

		OriginalPCMData = {};
		VisemeInfoPlayback = {};

		bUseFallback = false;
		TimeToGiveUp = 0.f;
		ExpectedRemainingAudio = 0;
		GotPackets = 0;
		bHasStartedProcessingAudio = false;
		bIsActive = true;

		FWaveModInfo WaveInfo;
		if (!WaveInfo.ReadWaveInfo(Message.SoundData.GetData(), Message.SoundData.Num()))
		{
			return;
		}

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

			ExpectedRemainingAudio = PCMData.Num() * 2;
			OriginalPCMData = {};
			OriginalPCMData.SetNumUninitialized(ExpectedRemainingAudio);
			FMemory::Memcpy(OriginalPCMData.GetData(), PCMData.GetData(), ExpectedRemainingAudio);
			VisemeInfoPlayback = {};

			FCharacterUtteranceVisemeInfo CurrentVisemeInfo;
			FCharacterUtteranceVisemeInfo PreviousVisemeInfo;

			for (const auto& VisemeInfo : Message.VisemeInfos)
			{
				if (!VisemeInfo.Code.IsEmpty())
				{
					VisemeInfoPlayback.Add(VisemeInfo);
				}
			}
		}

		while (!Message.A2FData->PendingAudio.IsEmpty())
		{
			TArray<uint8> Audio;
			Message.A2FData->PendingAudio.Dequeue(Audio);
			AudioToPlay.Enqueue(Audio);
			ExpectedRemainingAudio -= Audio.Num();
			if (ExpectedRemainingAudio > 0)
			{
				FMemory::Memcpy(OriginalPCMData.GetData(), OriginalPCMData.GetData() + Audio.Num(), ExpectedRemainingAudio);
			}
			OriginalPCMData.SetNum(FMath::Max(0, ExpectedRemainingAudio));
			GotPackets++;
		}
		while (!Message.A2FData->PendingBlendShapeMap.IsEmpty())
		{
			TMap<FName, float> BlendShapeMap;
			Message.A2FData->PendingBlendShapeMap.Dequeue(BlendShapeMap);
			AnimsToPlay.Enqueue(BlendShapeMap);
		}

		LockMessageQueue();

		if (GotPackets < 5)
		{
			TimeToGiveUp = AllowedLatencyDelay;
		}
		else if (!AudioComponent->IsActive())
		{
			AudioComponent->Play();
		}
	}
}

void UInworldCharacterPlaybackA2F::OnCharacterUtteranceInterrupt_Implementation(const FCharacterMessageUtterance& Message)
{
	FScopeLock ScopeLock(&QueueLock);

	if (A2FData)
	{
		A2FData->OnA2FOldAnimationHeaderData.Remove(HeaderDataHandle);
		A2FData->OnA2FOldAnimationContentData.Remove(ContentDataHandle);
	}

	AudioToPlay.Empty();
	AnimsToPlay.Empty();

	OriginalPCMData = {};
	VisemeInfoPlayback = {};

	bUseFallback = false;
	TimeToGiveUp = 0.f;
	ExpectedRemainingAudio = 0;
	GotPackets = 0;
	bHasStartedProcessingAudio = false;
	bIsActive = false;

	SoundStreaming->ResetAudio();
	OnUtteranceInterrupted.Broadcast();
	OnUtteranceStopped.Broadcast();
	UnlockMessageQueue();
}

void UInworldCharacterPlaybackA2F::OnA2FOldAnimationHeaderData(const FInworldA2FOldAnimationHeaderEvent& AnimationHeaderData)
{
	// Do Nothing...
}

void UInworldCharacterPlaybackA2F::OnA2FOldAnimationContentData(const FInworldA2FOldAnimationContentEvent& AnimationData)
{
	FScopeLock ScopeLock(&QueueLock);
	if (AnimationData.Audio.IsEmpty() || AnimationData.BlendShapeMap.IsEmpty() || bUseFallback)
	{
		return;
	}

	{
		AudioToPlay.Enqueue(AnimationData.Audio);
		AnimsToPlay.Enqueue(AnimationData.BlendShapeMap);
	}

	ExpectedRemainingAudio -= AnimationData.Audio.Num();
	GotPackets++;

	if (ExpectedRemainingAudio > 0)
	{
		FMemory::Memcpy(OriginalPCMData.GetData(), OriginalPCMData.GetData() + AnimationData.Audio.Num(), ExpectedRemainingAudio);
	}
	OriginalPCMData.SetNum(FMath::Max(0, ExpectedRemainingAudio));

	if (!AudioComponent->IsActive())
	{
		AudioComponent->Play();
	}
}

void UInworldCharacterPlaybackA2F::GenerateData(USoundWaveProcedural* InProceduralWave, int32 SamplesRequired)
{
	if (!bIsActive)
	{
		return;
	}
	if (TimeToGiveUp > 0.f && !bHasStartedProcessingAudio && ExpectedRemainingAudio > 0 && GotPackets < 10)
	{
		return;
	}

	FScopeLock ScopeLock(&QueueLock);
	bHasStartedProcessingAudio = true;
	if (AudioToPlay.IsEmpty() && OriginalPCMData.Num() > 0 && false)
	{
		bUseFallback = true;
		const int32 DataPerFrame = 1066;
		while (!OriginalPCMData.IsEmpty())
		{
			TArray<uint8> AudioData = OriginalPCMData;
			AudioData.SetNum(DataPerFrame);
			ExpectedRemainingAudio -= AudioData.Num();

			if (ExpectedRemainingAudio > 0)
			{
				FMemory::Memcpy(OriginalPCMData.GetData(), OriginalPCMData.GetData() + AudioData.Num(), ExpectedRemainingAudio);
			}
			OriginalPCMData.SetNum(FMath::Max(0, ExpectedRemainingAudio));
			AudioToPlay.Enqueue(AudioData);
		}
	}

	TArray<uint8> ToQueue;
	if (AudioToPlay.Dequeue(ToQueue))
	{
		InProceduralWave->QueueAudio(ToQueue.GetData(), ToQueue.Num());
		TMap<FName, float> AnimToPlay;
		AnimsToPlay.Dequeue(AnimToPlay);
		AsyncTask(ENamedThreads::GameThread, [this, AnimToPlay]()
			{
				FA2FBlendShapeData Data;
				Data.Map = AnimToPlay;
				OnInworldAudio2FaceBlendShapeUpdate.Broadcast(Data);
			});
	}
	else if (ExpectedRemainingAudio <= 0)
	{
		AsyncTask(ENamedThreads::GameThread, [this]()
			{
				FScopeLock ScopeLock(&QueueLock);
				if (A2FData)
				{
					A2FData->OnA2FOldAnimationHeaderData.Remove(HeaderDataHandle);
					A2FData->OnA2FOldAnimationContentData.Remove(ContentDataHandle);
				}
				AudioToPlay.Empty();
				AnimsToPlay.Empty();

				OriginalPCMData = {};
				VisemeInfoPlayback = {};

				bUseFallback = false;
				TimeToGiveUp = 0.f;
				ExpectedRemainingAudio = 0;
				GotPackets = 0;
				bHasStartedProcessingAudio = false;
				bIsActive = false;
				SoundStreaming->ResetAudio();
				OnUtteranceStopped.Broadcast();
				UnlockMessageQueue();
			});
	}
}