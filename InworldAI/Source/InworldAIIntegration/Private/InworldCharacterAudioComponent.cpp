/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */


#include "InworldCharacterAudioComponent.h"
#include "InworldCharacterComponent.h"

#include "Sound/SoundWave.h"
#include "Sound/SoundWaveProcedural.h"
#include "TimerManager.h"

#include "Async/Async.h"
#include "Async/TaskGraphInterfaces.h"

#include <GameFramework/Actor.h>
#include <Engine/World.h>

void UInworldCharacterAudioComponent::BeginPlay()
{
	Super::BeginPlay();

	SoundStreaming = NewObject<USoundWaveProcedural>();
	SoundStreaming->Duration = INDEFINITELY_LOOPING_DURATION;
	SoundStreaming->SoundGroup = SOUNDGROUP_Voice;
	SoundStreaming->bLooping = false;

	SoundStreaming->OnSoundWaveProceduralUnderflow = FOnSoundWaveProceduralUnderflow::CreateUObject(this, &UInworldCharacterAudioComponent::GenerateData);

	SetSound(SoundStreaming);

	CharacterComponent = Cast<UInworldCharacterComponent>(GetOwner()->GetComponentByClass(UInworldCharacterComponent::StaticClass()));
	if (CharacterComponent.IsValid())
	{
		CharacterComponent->OnUtterance.AddDynamic(this, &UInworldCharacterAudioComponent::OnCharacterUtterance);
		CharacterComponent->OnUtteranceInterrupt.AddDynamic(this, &UInworldCharacterAudioComponent::OnCharacterUtteranceInterrupt);
		CharacterComponent->OnUtterancePause.AddDynamic(this, &UInworldCharacterAudioComponent::OnCharacterUtterancePause);
		CharacterComponent->OnUtteranceResume.AddDynamic(this, &UInworldCharacterAudioComponent::OnCharacterUtteranceResume);
		CharacterComponent->OnSilence.AddDynamic(this, &UInworldCharacterAudioComponent::OnCharacterSilence);
		CharacterComponent->OnSilenceInterrupt.AddDynamic(this, &UInworldCharacterAudioComponent::OnCharacterSilenceInterrupt);
	}
}

void UInworldCharacterAudioComponent::OnCharacterUtterance(const FCharacterMessageUtterance& Message)
{
	FScopeLock ScopeLock(&QueueLock);

	UtteranceData = Message.UtteranceData;

	NumSoundDataBytesPlayed = 44;

	SoundStreaming->NumChannels = UtteranceData->ChannelCount;
	SoundStreaming->SetSampleRate(UtteranceData->SamplesPerSecond);

	if (bIsPaused)
	{
		SetPaused(false);
	}

	Play();

	CharacterComponent->LockMessageQueue(CharacterMessageQueueLockHandle);
}

void UInworldCharacterAudioComponent::OnCharacterUtteranceInterrupt(const FCharacterMessageUtterance& Message)
{
	Stop();
	UtteranceData = nullptr;
	OnVisemeBlendsUpdated.Broadcast({});
	CharacterComponent->UnlockMessageQueue(CharacterMessageQueueLockHandle);
}

void UInworldCharacterAudioComponent::OnCharacterUtterancePause(const FCharacterMessageUtterance& Message)
{
	SetPaused(true);
}

void UInworldCharacterAudioComponent::OnCharacterUtteranceResume(const FCharacterMessageUtterance& Message)
{
	SetPaused(false);
}

void UInworldCharacterAudioComponent::OnCharacterSilence(const FCharacterMessageSilence& Message)
{
	GetWorld()->GetTimerManager().SetTimer(SilenceTimerHandle, this, &UInworldCharacterAudioComponent::OnSilenceEnd, Message.Duration);
	CharacterComponent->LockMessageQueue(CharacterMessageQueueLockHandle);
}

void UInworldCharacterAudioComponent::OnCharacterSilenceInterrupt(const FCharacterMessageSilence& Message)
{
	GetWorld()->GetTimerManager().ClearTimer(SilenceTimerHandle);
	CharacterComponent->UnlockMessageQueue(CharacterMessageQueueLockHandle);
}

void UInworldCharacterAudioComponent::OnSilenceEnd()
{
	CharacterComponent->UnlockMessageQueue(CharacterMessageQueueLockHandle);
}

void UInworldCharacterAudioComponent::GenerateData(USoundWaveProcedural* InProceduralWave, int32 SamplesRequired)
{
	FScopeLock ScopeLock(&QueueLock);
	if (UtteranceData == nullptr || UtteranceData->SoundData.Num() == 0)
	{
		return;
	}
	if (NumSoundDataBytesPlayed == UtteranceData->SoundData.Num() && UtteranceData->bAudioFinal)
	{
		NumSoundDataBytesPlayed = 44;
		SoundStreaming->ResetAudio();
		AsyncTask(ENamedThreads::GameThread, [this]()
			{
				if (!GetOwner()->IsPendingKillPending())
				{
					FScopeLock ScopeLock(&QueueLock);
					OnAudioFinished();
				}
			});
	}
	else
	{
		const int NextSampleCount = FMath::Min(SamplesRequired, UtteranceData->SoundData.Num() - NumSoundDataBytesPlayed);
		InProceduralWave->QueueAudio(UtteranceData->SoundData.GetData() + NumSoundDataBytesPlayed, NextSampleCount);
		NumSoundDataBytesPlayed += NextSampleCount;
		AsyncTask(ENamedThreads::GameThread, [this]()
			{
				if (!GetOwner()->IsPendingKillPending())
				{
					FScopeLock ScopeLock(&QueueLock);
					if(!UtteranceData)
					{
						return;
					}
					if (UtteranceData->IsType<FCharacterMessageUtteranceDataInworld>())
					{
						UpdateVisemeBlends();
					}
					else if (UtteranceData->IsType<FCharacterMessageUtteranceDataA2F>())
					{
						UpdateBlendShapes();
					}
				}
			});
	}
}

float UInworldCharacterAudioComponent::GetAudioDuration() const
{
	if (!UtteranceData)
	{
		return 0.f;
	}
	const int32 SoundDataSize = UtteranceData->SoundData.Num();
	const int32 ChannelCount = UtteranceData->ChannelCount;
	const int32 BitsPerSample = UtteranceData->BitsPerSample;
	const int32 SamplesPerSecond = UtteranceData->SamplesPerSecond;
	return (float)SoundDataSize / ((float)ChannelCount * ((float)BitsPerSample / 8.f) * (float)SamplesPerSecond);
}

float UInworldCharacterAudioComponent::GetAudioPlaybackPercent() const
{
	const float AudioDuration = GetAudioDuration();
	return AudioDuration > 0.f ? GetElapsedTimeForCurrentUtterance() / AudioDuration : 0.f;
}

float UInworldCharacterAudioComponent::GetElapsedTimeForCurrentUtterance() const
{
	if (!UtteranceData)
	{
		return 0.f;
	}
	const int32 SoundDataSize = NumSoundDataBytesPlayed - 44;
	const int32 ChannelCount = UtteranceData->ChannelCount;
	const int32 BitsPerSample = UtteranceData->BitsPerSample;
	const int32 SamplesPerSecond = UtteranceData->SamplesPerSecond;
	return (float)SoundDataSize / ((float)ChannelCount * ((float)BitsPerSample / 8.f) * (float)SamplesPerSecond);
}

float UInworldCharacterAudioComponent::GetRemainingTimeForCurrentUtterance() const
{
	return GetAudioDuration() - GetElapsedTimeForCurrentUtterance();
}

void UInworldCharacterAudioComponent::UpdateVisemeBlends()
{
	TSharedPtr<FCharacterMessageUtteranceDataInworld> UtteranceDataInworld = StaticCastSharedPtr<FCharacterMessageUtteranceDataInworld>(UtteranceData);
	ensure(UtteranceDataInworld);

	TArray<FCharacterUtteranceVisemeInfo>& VisemeInfos = UtteranceDataInworld->VisemeInfos;

	FInworldCharacterVisemeBlends VisemeBlends = {};

	const float CurrentAudioPlaybackTime = GetAudioPlaybackPercent() * GetAudioDuration();

	FCharacterUtteranceVisemeInfo CurrentVisemeInfo = {};
	FCharacterUtteranceVisemeInfo PreviousVisemeInfo = {};

	{
		const int32 INVALID_INDEX = -1;
		int32 Target = INVALID_INDEX;
		int32 L = 0;
		int32 R = VisemeInfos.Num() - 1;
		while (L <= R)
		{
			const int32 Mid = (L + R) >> 1;
			const FCharacterUtteranceVisemeInfo& Sample = VisemeInfos[Mid];
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
		if (VisemeInfos.IsValidIndex(Target))
		{
			CurrentVisemeInfo = VisemeInfos[Target];
		}
		if (VisemeInfos.IsValidIndex(Target - 1))
		{
			PreviousVisemeInfo = VisemeInfos[Target - 1];
		}
	}

	const float Blend = (CurrentAudioPlaybackTime - PreviousVisemeInfo.Timestamp) / (CurrentVisemeInfo.Timestamp - PreviousVisemeInfo.Timestamp);

	VisemeBlends.STOP = 0.f;
	VisemeBlends[PreviousVisemeInfo.Code] = FMath::Clamp(1.f - Blend, 0.f, 1.f);
	VisemeBlends[CurrentVisemeInfo.Code] = FMath::Clamp(Blend, 0.f, 1.f);

	OnVisemeBlendsUpdated.Broadcast(VisemeBlends);
}

void UInworldCharacterAudioComponent::UpdateBlendShapes()
{
	TSharedPtr<FCharacterMessageUtteranceDataA2F> UtteranceDataA2F = StaticCastSharedPtr<FCharacterMessageUtteranceDataA2F>(UtteranceData);
	ensure(UtteranceDataA2F);

	const float ElapsedTime = GetElapsedTimeForCurrentUtterance();
	const float CurrentFrame = ElapsedTime / (1.f / 30.f);
	const int32 PrevFrameIndex = FMath::Clamp<int32>(FMath::FloorToInt(CurrentFrame), 0, UtteranceDataA2F->BlendShapeMaps.Num() - 1);
	const int32 NextFrameIndex = FMath::Clamp<int32>(FMath::CeilToInt(CurrentFrame), 0, UtteranceDataA2F->BlendShapeMaps.Num() - 1);
	double tmp;
	const float NextFrameWeight = FMath::Modf(CurrentFrame, &tmp);
	const float PrevFrameWeight = 1.f - NextFrameWeight;

	TMap<FName, float> PrevFrame = UtteranceDataA2F->BlendShapeMaps[PrevFrameIndex];
	TMap<FName, float> NextFrame = UtteranceDataA2F->BlendShapeMaps[NextFrameIndex];

	TMap<FName, float> BlendShapes;
	for (const FName& BlendShapeName : UtteranceDataA2F->BlendShapeNames)
	{
		BlendShapes.Add(BlendShapeName, (PrevFrame[BlendShapeName] * PrevFrameWeight) + (NextFrame[BlendShapeName] * NextFrameWeight));
	}
	OnBlendShapesUpdated.Broadcast({ BlendShapes });
}

void UInworldCharacterAudioComponent::OnAudioFinished()
{
	UtteranceData = nullptr;
	OnVisemeBlendsUpdated.Broadcast({});
	CharacterComponent->UnlockMessageQueue(CharacterMessageQueueLockHandle);
}
