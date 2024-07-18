/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */


#include "InworldCharacterAudioComponent.h"
#include "InworldCharacterComponent.h"
#include "Audio.h"
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
	if (!Message.bAudioFinal)
	{
		return;
	}

	FWaveModInfo WaveInfo;
	if (!WaveInfo.ReadWaveInfo(Message.SoundData.GetData(), Message.SoundData.Num()))
	{
		return;
	}

	FScopeLock ScopeLock(&QueueLock);
	SoundDataSize = Message.SoundData.Num() - 44;
	SoundDataPlayed = 0;
	SoundData = TArray<uint8>(Message.SoundData.GetData() + 44, SoundDataSize);

	SoundDuration = *WaveInfo.pWaveDataSize / (*WaveInfo.pChannels * (*WaveInfo.pBitsPerSample / 8.f) * *WaveInfo.pSamplesPerSec);
	CurrentAudioPlaybackPercent = 0.f;

	SoundStreaming->NumChannels = *WaveInfo.pChannels;
	SoundStreaming->SetSampleRate(*WaveInfo.pSamplesPerSec);

	VisemeInfoPlayback.Empty();
	VisemeInfoPlayback.Reserve(Message.VisemeInfos.Num());

	CurrentVisemeInfo = FCharacterUtteranceVisemeInfo();
	PreviousVisemeInfo = FCharacterUtteranceVisemeInfo();
	VisemeInfoPlayback.Add({ TEXT("STOP"), 0.f });
	for (const auto& VisemeInfo : Message.VisemeInfos)
	{
		if (!VisemeInfo.Code.IsEmpty())
		{
			VisemeInfoPlayback.Add(VisemeInfo);
		}
	}
	VisemeInfoPlayback.Add({ TEXT("STOP"), SoundDuration });

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
	VisemeBlends = FInworldCharacterVisemeBlends();
	OnVisemeBlendsUpdated.Broadcast(VisemeBlends);
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
	if (SoundData.Num() == 0)
	{
		return;
	}
	if (SoundDataPlayed == SoundDataSize)
	{
		SoundData = {};
		SoundDataPlayed = 0;
		SoundDataSize = 0;
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
		const int NextSampleCount = FMath::Min(SamplesRequired, SoundDataSize - SoundDataPlayed);
		InProceduralWave->QueueAudio(SoundData.GetData() + SoundDataPlayed, NextSampleCount);
		SoundDataPlayed += NextSampleCount;
		AsyncTask(ENamedThreads::GameThread, [this]()
			{
				if (!GetOwner()->IsPendingKillPending())
				{
					FScopeLock ScopeLock(&QueueLock);
					OnAudioPlaybackPercent();
				}
			});
	}
}

float UInworldCharacterAudioComponent::GetRemainingTimeForCurrentUtterance() const
{
	if (Sound != nullptr || !IsPlaying())
	{
		return 0.f;
	}

	return (1.f - CurrentAudioPlaybackPercent) * SoundDuration;
}

void UInworldCharacterAudioComponent::OnAudioPlaybackPercent()
{
	const float Percent = SoundDataSize != 0 ? (float)SoundDataPlayed / (float)SoundDataSize : 0.f;
	CurrentAudioPlaybackPercent = Percent;

	VisemeBlends = FInworldCharacterVisemeBlends();

	const float CurrentAudioPlaybackTime = Percent * SoundDuration;

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
	}

	const float Blend = (CurrentAudioPlaybackTime - PreviousVisemeInfo.Timestamp) / (CurrentVisemeInfo.Timestamp - PreviousVisemeInfo.Timestamp);

	VisemeBlends.STOP = 0.f;
	VisemeBlends[PreviousVisemeInfo.Code] = FMath::Clamp(1.f - Blend, 0.f, 1.f);
	VisemeBlends[CurrentVisemeInfo.Code] = FMath::Clamp(Blend, 0.f, 1.f);

	OnVisemeBlendsUpdated.Broadcast(VisemeBlends);
}

void UInworldCharacterAudioComponent::OnAudioFinished()
{
	VisemeBlends = FInworldCharacterVisemeBlends();
	OnVisemeBlendsUpdated.Broadcast(VisemeBlends);
	CharacterComponent->UnlockMessageQueue(CharacterMessageQueueLockHandle);
}
