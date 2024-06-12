/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */


#include "InworldCharacterAudioComponent.h"
#include "InworldCharacterComponent.h"
#include "InworldBlueprintFunctionLibrary.h"
#include "Sound/SoundWave.h"
#include "TimerManager.h"

#include <GameFramework/Actor.h>
#include <Engine/World.h>

void UInworldCharacterAudioComponent::BeginPlay()
{
	Super::BeginPlay();

	CharacterComponent = Cast<UInworldCharacterComponent>(GetOwner()->GetComponentByClass(UInworldCharacterComponent::StaticClass()));
	if (CharacterComponent.IsValid())
	{
		AudioPlaybackPercentHandle = OnAudioPlaybackPercentNative.AddUObject(this, &UInworldCharacterAudioComponent::OnAudioPlaybackPercent);
		AudioFinishedHandle = OnAudioFinishedNative.AddUObject(this, &UInworldCharacterAudioComponent::OnAudioFinished);

		CharacterComponent->OnUtterance.AddDynamic(this, &UInworldCharacterAudioComponent::OnCharacterUtterance);
		CharacterComponent->OnUtteranceInterrupt.AddDynamic(this, &UInworldCharacterAudioComponent::OnCharacterUtteranceInterrupt);
		CharacterComponent->OnSilence.AddDynamic(this, &UInworldCharacterAudioComponent::OnCharacterSilence);
		CharacterComponent->OnSilenceInterrupt.AddDynamic(this, &UInworldCharacterAudioComponent::OnCharacterSilenceInterrupt);
	}
}

void UInworldCharacterAudioComponent::OnCharacterUtterance(const FCharacterMessageUtterance& Message)
{
	SetSound(nullptr);
	CurrentAudioPlaybackPercent = 0.f;
	SoundDuration = 0.f;
	if (Message.SoundData.Num() > 0 && Message.bAudioFinal)
	{
		USoundWave* UtteranceSoundWave = UInworldBlueprintFunctionLibrary::DataArrayToSoundWave(Message.SoundData);
		SoundDuration = UtteranceSoundWave->GetDuration();
		SetSound(UtteranceSoundWave);

		VisemeInfoPlayback.Empty();
		VisemeInfoPlayback.Reserve(Message.VisemeInfos.Num());

		CurrentVisemeInfo = FCharacterUtteranceVisemeInfo();
		PreviousVisemeInfo = FCharacterUtteranceVisemeInfo();
		for (const auto& VisemeInfo : Message.VisemeInfos)
		{
			if (!VisemeInfo.Code.IsEmpty())
			{
				VisemeInfoPlayback.Add(VisemeInfo);
			}
		}

		Play();

		CharacterComponent->MakeMessageQueueLock(CharacterMessageQueueLockHandle);
	}
}

void UInworldCharacterAudioComponent::OnCharacterUtteranceInterrupt(const FCharacterMessageUtterance& Message)
{
	Stop();
	VisemeBlends = FInworldCharacterVisemeBlends();
	OnVisemeBlendsUpdated.Broadcast(VisemeBlends);
}

void UInworldCharacterAudioComponent::OnCharacterSilence(const FCharacterMessageSilence& Message)
{
	GetWorld()->GetTimerManager().SetTimer(SilenceTimerHandle, this, &UInworldCharacterAudioComponent::OnSilenceEnd, Message.Duration);
	CharacterComponent->MakeMessageQueueLock(CharacterMessageQueueLockHandle);
}

void UInworldCharacterAudioComponent::OnCharacterSilenceInterrupt(const FCharacterMessageSilence& Message)
{
	GetWorld()->GetTimerManager().ClearTimer(SilenceTimerHandle);
	CharacterComponent->ClearMessageQueueLock(CharacterMessageQueueLockHandle);
}

void UInworldCharacterAudioComponent::OnSilenceEnd()
{
	CharacterComponent->ClearMessageQueueLock(CharacterMessageQueueLockHandle);
}

float UInworldCharacterAudioComponent::GetRemainingTimeForCurrentUtterance() const
{
	if (Sound != nullptr || !IsPlaying())
	{
		return 0.f;
	}

	return (1.f - CurrentAudioPlaybackPercent) * Sound->Duration;
}

void UInworldCharacterAudioComponent::OnAudioPlaybackPercent(const UAudioComponent* InAudioComponent, const USoundWave* InSoundWave, float Percent)
{
	CurrentAudioPlaybackPercent = Percent;

	VisemeBlends = FInworldCharacterVisemeBlends();

	const float CurrentAudioPlaybackTime = SoundDuration * Percent;

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

void UInworldCharacterAudioComponent::OnAudioFinished(UAudioComponent* InAudioComponent)
{
	VisemeBlends = FInworldCharacterVisemeBlends();
	OnVisemeBlendsUpdated.Broadcast(VisemeBlends);
	CharacterComponent->ClearMessageQueueLock(CharacterMessageQueueLockHandle);
}
