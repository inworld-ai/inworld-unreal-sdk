/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */


#include "InworldCharacterA2FComponent.h"
#include "InworldCharacterComponent.h"
#include "InworldBlueprintFunctionLibrary.h"
#include "AudioResampler.h"
#include "Sound/SoundWave.h"
#include "Sound/SoundWaveProcedural.h"
#include "TimerManager.h"

UInworldCharacterA2FComponent::UInworldCharacterA2FComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UInworldCharacterA2FComponent::BeginPlay()
{
	Super::BeginPlay();

	SoundStreaming = NewObject<USoundWaveProcedural>();
	SoundStreaming->SetSampleRate(22050);
	SoundStreaming->NumChannels = 1;
	SoundStreaming->Duration = INDEFINITELY_LOOPING_DURATION;
	SoundStreaming->SoundGroup = SOUNDGROUP_Voice;
	SoundStreaming->bLooping = false;

	SoundStreaming->OnSoundWaveProceduralUnderflow = FOnSoundWaveProceduralUnderflow::CreateUObject(this, &UInworldCharacterA2FComponent::GenerateData);

	SetSound(SoundStreaming);

	CharacterComponent = Cast<UInworldCharacterComponent>(GetOwner()->GetComponentByClass(UInworldCharacterComponent::StaticClass()));
	if (CharacterComponent.IsValid())
	{
		CharacterComponent->OnUtterance.AddDynamic(this, &UInworldCharacterA2FComponent::OnCharacterUtterance);
		CharacterComponent->OnUtteranceInterrupt.AddDynamic(this, &UInworldCharacterA2FComponent::OnCharacterUtteranceInterrupt);
	}
}

void UInworldCharacterA2FComponent::EndPlay(EEndPlayReason::Type Reason)
{
	Super::EndPlay(Reason);

	SoundStreaming->ResetAudio();
	OnUtteranceInterrupted.Broadcast();
	OnUtteranceStopped.Broadcast();
	CharacterComponent->ClearMessageQueueLock(CharacterMessageQueueLockHandle);
}

void UInworldCharacterA2FComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TimeToGiveUp -= DeltaTime;
	if (!IsActive())
	{
		Play();
	}
}

void UInworldCharacterA2FComponent::OnCharacterUtterance(const FCharacterMessageUtterance& Message)
{

	if (Message.SoundData.Num() > 0 && Message.bAudioFinal)
	{
		A2FData = Message.A2FData;
		A2FDataUpdateHandle = A2FData->OnCharacterMessageUtteranceA2FDataUpdate.AddUObject(this, &UInworldCharacterA2FComponent::OnCharacterMessageUtteranceA2FDataUpdate);

		FScopeLock ScopeLock(&QueueLock);
		AudioToPlay.Empty();
		AnimsToPlay.Empty();

		OriginalPCMData = {};
		VisemeInfoPlayback = {};

		bUseFallback = false;
		TimeToGiveUp = AllowedLatencyDelay;
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
			22050.f,
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

			SoundDuration = UInworldBlueprintFunctionLibrary::DataArrayToSoundWave(Message.SoundData)->GetDuration();
			SoundSize = PCMData.Num() * 2;
			ExpectedRemainingAudio = SoundSize;
			OriginalPCMData = {};
			OriginalPCMData.SetNumUninitialized(SoundSize);
			FMemory::Memcpy(OriginalPCMData.GetData(), PCMData.GetData(), SoundSize);
			VisemeInfoPlayback = {};

			for (const auto& VisemeInfo : Message.VisemeInfos)
			{
				if (!VisemeInfo.Code.IsEmpty())
				{
					VisemeInfoPlayback.Add(VisemeInfo);
				}
			}
		}
		OnCharacterMessageUtteranceA2FDataUpdate();

		CharacterComponent->MakeMessageQueueLock(CharacterMessageQueueLockHandle);

		TimeToGiveUp = AllowedLatencyDelay;
		if (!IsActive())
		{
			Play();
		}
	}
}

void UInworldCharacterA2FComponent::OnCharacterUtteranceInterrupt(const FCharacterMessageUtterance& Message)
{
	FScopeLock ScopeLock(&QueueLock);

	if (A2FData)
	{
		A2FData->OnCharacterMessageUtteranceA2FDataUpdate.Remove(A2FDataUpdateHandle);
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
	CharacterComponent->ClearMessageQueueLock(CharacterMessageQueueLockHandle);
}

void UInworldCharacterA2FComponent::OnCharacterMessageUtteranceA2FDataUpdate()
{
	FScopeLock ScopeLock(&QueueLock);

	while (!A2FData->PendingAudio.IsEmpty())
	{
		TArray<uint8> Audio;
		A2FData->PendingAudio.Dequeue(Audio);
		AudioToPlay.Enqueue(Audio);
		ExpectedRemainingAudio -= Audio.Num();
		if (ExpectedRemainingAudio > 0)
		{
			FMemory::Memcpy(OriginalPCMData.GetData(), OriginalPCMData.GetData() + Audio.Num(), ExpectedRemainingAudio);
		}
		OriginalPCMData.SetNum(FMath::Max(0, ExpectedRemainingAudio));
		GotPackets++;
	}
	while (!A2FData->PendingBlendShapeMap.IsEmpty())
	{
		TMap<FName, float> BlendShapeMap;
		A2FData->PendingBlendShapeMap.Dequeue(BlendShapeMap);
		AnimsToPlay.Enqueue(BlendShapeMap);
	}

	if (!IsActive())
	{
		Play();
	}
}

void UInworldCharacterA2FComponent::GenerateData(USoundWaveProcedural* InProceduralWave, int32 SamplesRequired)
{
	if (!bIsActive)
	{
		return;
	}
	if (TimeToGiveUp > 0.f && !bHasStartedProcessingAudio && !A2FData->bIsDone && GotPackets < MinPacketsToStart)
	{
		return;
	}

	FScopeLock ScopeLock(&QueueLock);
	bHasStartedProcessingAudio = true;
	if (AudioToPlay.IsEmpty() && OriginalPCMData.Num() > 0)
	{
		bUseFallback = true;
		const int32 DataPerFrame = 1066;
		while (!OriginalPCMData.IsEmpty())
		{
			const float CurrentTime = (1.f - (OriginalPCMData.Num() / SoundSize)) * SoundDuration;
			TArray<uint8> AudioData = OriginalPCMData;
			AudioData.SetNum(DataPerFrame);
			ExpectedRemainingAudio -= AudioData.Num();
			A2FData->bIsDone = true;

			if (ExpectedRemainingAudio > 0)
			{
				FMemory::Memcpy(OriginalPCMData.GetData(), OriginalPCMData.GetData() + AudioData.Num(), ExpectedRemainingAudio);
			}
			OriginalPCMData.SetNum(FMath::Max(0, ExpectedRemainingAudio));
			AudioToPlay.Enqueue(AudioData);

			const int32 INVALID_INDEX = -1;
			int32 Target = INVALID_INDEX;
			int32 L = 0;
			int32 R = VisemeInfoPlayback.Num() - 1;
			while (L <= R)
			{
				const int32 Mid = (L + R) >> 1;
				const FCharacterUtteranceVisemeInfo& Sample = VisemeInfoPlayback[Mid];
				if (CurrentTime > Sample.Timestamp)
				{
					L = Mid + 1;
				}
				else
				{
					Target = Mid;
					R = Mid - 1;
				}
			}

			FCharacterUtteranceVisemeInfo CurrentVisemeInfo;
			FCharacterUtteranceVisemeInfo PreviousVisemeInfo;

			if (VisemeInfoPlayback.IsValidIndex(Target))
			{
				CurrentVisemeInfo = VisemeInfoPlayback[Target];
			}
			if (VisemeInfoPlayback.IsValidIndex(Target - 1))
			{
				PreviousVisemeInfo = VisemeInfoPlayback[Target - 1];
			}

			const float Blend = (CurrentTime - PreviousVisemeInfo.Timestamp) / (CurrentVisemeInfo.Timestamp - PreviousVisemeInfo.Timestamp);

			FInworldCharacterVisemeBlends VisemeBlends;
			VisemeBlends[PreviousVisemeInfo.Code] = FMath::Clamp(1.f - Blend, 0.f, 1.f);
			VisemeBlends[CurrentVisemeInfo.Code] = FMath::Clamp(Blend, 0.f, 1.f);

			BackupAnimsToPlay.Enqueue(VisemeBlends);
		}
	}

	TArray<uint8> ToQueue;
	if (AudioToPlay.Dequeue(ToQueue))
	{
		InProceduralWave->QueueAudio(ToQueue.GetData(), ToQueue.Num());
		if (bUseFallback)
		{
			FInworldCharacterVisemeBlends VisemeBlend;
			BackupAnimsToPlay.Dequeue(VisemeBlend);
			AsyncTask(ENamedThreads::GameThread, [this, VisemeBlend]()
				{
					OnInworldAudio2FaceBlendShapeBackupUpdate.Broadcast(VisemeBlend);
				});
		}
		else
		{
			TMap<FName, float> AnimToPlay;
			AnimsToPlay.Dequeue(AnimToPlay);
			AsyncTask(ENamedThreads::GameThread, [this, AnimToPlay]()
				{
					FA2FBlendShapeData Data;
					Data.Map = AnimToPlay;
					OnInworldAudio2FaceBlendShapeUpdate.Broadcast(Data);
				});
		}
	}
	else if (A2FData->bIsDone)
	{
		AsyncTask(ENamedThreads::GameThread, [this]()
			{
				FScopeLock ScopeLock(&QueueLock);
				if (A2FData)
				{
					A2FData->OnCharacterMessageUtteranceA2FDataUpdate.Remove(A2FDataUpdateHandle);
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

				CharacterComponent->ClearMessageQueueLock(CharacterMessageQueueLockHandle);
			});
	}
}
