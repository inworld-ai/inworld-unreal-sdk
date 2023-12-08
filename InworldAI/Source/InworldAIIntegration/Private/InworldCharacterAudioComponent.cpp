/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */


#include "InworldCharacterAudioComponent.h"
#include "InworldCharacterComponent.h"
#include "InworldBlueprintFunctionLibrary.h"

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
	if (Sound == nullptr || !IsPlaying())
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
	//*VisemeBlends[PreviousVisemeInfo.Code] = FMath::Clamp(1.f - Blend, 0.f, 1.f);
	//*VisemeBlends[CurrentVisemeInfo.Code] = FMath::Clamp(Blend, 0.f, 1.f);

	if ("PP" == PreviousVisemeInfo.Code)
	{
		VisemeBlends.PP = FMath::Clamp(1.f - Blend, 0.f, 1.f);
	}
	if ("FF" == PreviousVisemeInfo.Code)
	{
		VisemeBlends.FF = FMath::Clamp(1.f - Blend, 0.f, 1.f);
	}
	if ( "TH" == PreviousVisemeInfo.Code )
	{
		VisemeBlends.TH = FMath::Clamp(1.f - Blend, 0.f, 1.f);
	}
	if ( "DD" == PreviousVisemeInfo.Code )
	{
		VisemeBlends.DD = FMath::Clamp(1.f - Blend, 0.f, 1.f);
	}
	if ( "Kk" == PreviousVisemeInfo.Code )
	{
		VisemeBlends.Kk = FMath::Clamp(1.f - Blend, 0.f, 1.f);
	}
	if ( "CH" == PreviousVisemeInfo.Code )
	{
		VisemeBlends.CH = FMath::Clamp(1.f - Blend, 0.f, 1.f);
	}
	if ( "SS" == PreviousVisemeInfo.Code )
	{
		VisemeBlends.SS = FMath::Clamp(1.f - Blend, 0.f, 1.f);
	}
	if ("Nn" == PreviousVisemeInfo.Code)
	{
		VisemeBlends.Nn = FMath::Clamp(1.f - Blend, 0.f, 1.f);
	}
	if ("RR" == PreviousVisemeInfo.Code)
	{
		VisemeBlends.RR = FMath::Clamp(1.f - Blend, 0.f, 1.f);
	}
	if ("Aa" == PreviousVisemeInfo.Code)
	{
		VisemeBlends.Aa = FMath::Clamp(1.f - Blend, 0.f, 1.f);
	}
	if ("E" == PreviousVisemeInfo.Code)
	{
		VisemeBlends.E = FMath::Clamp(1.f - Blend, 0.f, 1.f);
	}
	if ("I" == PreviousVisemeInfo.Code)
	{
		VisemeBlends.I = FMath::Clamp(1.f - Blend, 0.f, 1.f);
	}
	if ("O" == PreviousVisemeInfo.Code)
	{
		VisemeBlends.O = FMath::Clamp(1.f - Blend, 0.f, 1.f);
	}
	if ("U" == PreviousVisemeInfo.Code)
	{
		VisemeBlends.U = FMath::Clamp(1.f - Blend, 0.f, 1.f);
	}



	if ("PP" == CurrentVisemeInfo.Code)
	{
		VisemeBlends.PP = FMath::Clamp(Blend, 0.f, 1.f);
	}
	if ("FF" == CurrentVisemeInfo.Code)
	{
		VisemeBlends.FF = FMath::Clamp(Blend, 0.f, 1.f);
	}
	if ("TH" == CurrentVisemeInfo.Code)
	{
		VisemeBlends.TH = FMath::Clamp(Blend, 0.f, 1.f);
	}
	if ("DD" == CurrentVisemeInfo.Code)
	{
		VisemeBlends.DD = FMath::Clamp(Blend, 0.f, 1.f);
	}
	if ("Kk" == CurrentVisemeInfo.Code)
	{
		VisemeBlends.Kk = FMath::Clamp(Blend, 0.f, 1.f);
	}
	if ("CH" == CurrentVisemeInfo.Code)
	{
		VisemeBlends.CH = FMath::Clamp(Blend, 0.f, 1.f);
	}
	if ("SS" == CurrentVisemeInfo.Code)
	{
		VisemeBlends.SS = FMath::Clamp(Blend, 0.f, 1.f);
	}
	if ("Nn" == CurrentVisemeInfo.Code)
	{
		VisemeBlends.Nn = FMath::Clamp(Blend, 0.f, 1.f);
	}
	if ("RR" == CurrentVisemeInfo.Code)
	{
		VisemeBlends.RR = FMath::Clamp(Blend, 0.f, 1.f);
	}
	if ("Aa" == CurrentVisemeInfo.Code)
	{
		VisemeBlends.Aa = FMath::Clamp(Blend, 0.f, 1.f);
	}
	if ("E" == CurrentVisemeInfo.Code)
	{
		VisemeBlends.E = FMath::Clamp(Blend, 0.f, 1.f);
	}
	if ("I" == CurrentVisemeInfo.Code)
	{
		VisemeBlends.I = FMath::Clamp(Blend, 0.f, 1.f);
	}
	if ("O" == CurrentVisemeInfo.Code)
	{
		VisemeBlends.O = FMath::Clamp(Blend, 0.f, 1.f);
	}
	if ("U" == CurrentVisemeInfo.Code)
	{
		VisemeBlends.U = FMath::Clamp(Blend, 0.f, 1.f);
	}

	OnVisemeBlendsUpdated.Broadcast(VisemeBlends);
}

void UInworldCharacterAudioComponent::OnAudioFinished(UAudioComponent* InAudioComponent)
{
	VisemeBlends = FInworldCharacterVisemeBlends();
	OnVisemeBlendsUpdated.Broadcast(VisemeBlends);
	CharacterComponent->ClearMessageQueueLock(CharacterMessageQueueLockHandle);
}
