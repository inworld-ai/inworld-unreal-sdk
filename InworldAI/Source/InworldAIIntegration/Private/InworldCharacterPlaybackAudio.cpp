/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldCharacterPlaybackAudio.h"
#include "InworldCharacterComponent.h"
#include "InworldBlueprintFunctionLibrary.h"
#include <Components/AudioComponent.h>

void UInworldCharacterPlaybackAudio::BeginPlay_Implementation()
{
	Super::BeginPlay_Implementation();

	AudioComponent = Cast<UAudioComponent>(OwnerActor->GetComponentByClass(UAudioComponent::StaticClass()));

	if (ensureMsgf(AudioComponent.IsValid(), TEXT("UInworldCharacterPlaybackAudio owner doesn't contain AudioComponent")))
	{
		AudioPlaybackPercentHandle = AudioComponent->OnAudioPlaybackPercentNative.AddUObject(this, &UInworldCharacterPlaybackAudio::OnAudioPlaybackPercent);
		AudioFinishedHandle = AudioComponent->OnAudioFinishedNative.AddUObject(this, &UInworldCharacterPlaybackAudio::OnAudioFinished);
	}
}

void UInworldCharacterPlaybackAudio::EndPlay_Implementation()
{
	Super::EndPlay_Implementation();

	if (AudioComponent.IsValid())
	{
		AudioComponent->OnAudioPlaybackPercentNative.Remove(AudioPlaybackPercentHandle);
		AudioComponent->OnAudioFinishedNative.Remove(AudioFinishedHandle);
	}
}

void UInworldCharacterPlaybackAudio::OnCharacterUtterance_Implementation(const FCharacterMessageUtterance& Message)
{
	SoundWave = nullptr;
	CurrentAudioPlaybackPercent = 0.f;
	SoundDuration = 0.f;
	if (Message.SoundData.Num() > 0 && Message.bAudioFinal)
	{
		SoundWave = UInworldBlueprintFunctionLibrary::DataArrayToSoundWave(Message.SoundData);
		SoundDuration = SoundWave->GetDuration();
		AudioComponent->SetSound(SoundWave);

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

		AudioComponent->Play();

		LockMessageQueue();
	}
	OnUtteranceStarted.Broadcast(SoundWave != nullptr ? SoundDuration : 0.f, Message.Text);
}

void UInworldCharacterPlaybackAudio::OnCharacterUtteranceInterrupt_Implementation(const FCharacterMessageUtterance& Message)
{
	AudioComponent->Stop();
	VisemeBlends = FInworldCharacterVisemeBlends();
	OnVisemeBlendsUpdated.Broadcast(VisemeBlends);
	OnUtteranceInterrupted.Broadcast();
}

void UInworldCharacterPlaybackAudio::OnCharacterSilence_Implementation(const FCharacterMessageSilence& Message)
{
	UWorld* World = CharacterComponent->GetWorld();
	World->GetTimerManager().SetTimer(SilenceTimerHandle, this, &UInworldCharacterPlaybackAudio::OnSilenceEnd, Message.Duration);
	OnSilenceStarted.Broadcast(Message.Duration);
	LockMessageQueue();
}

void UInworldCharacterPlaybackAudio::OnCharacterSilenceInterrupt_Implementation(const FCharacterMessageSilence& Message)
{
	UWorld* World = CharacterComponent->GetWorld();
	World->GetTimerManager().ClearTimer(SilenceTimerHandle);
}

void UInworldCharacterPlaybackAudio::OnSilenceEnd()
{
	OnSilenceStopped.Broadcast();
	UnlockMessageQueue();
}

float UInworldCharacterPlaybackAudio::GetRemainingTimeForCurrentUtterance() const
{
	if (!AudioComponent.IsValid() || !AudioComponent->Sound || !AudioComponent->IsPlaying())
	{
		return 0.f;
	}

	return (1.f - CurrentAudioPlaybackPercent) * AudioComponent->Sound->Duration;
}

void UInworldCharacterPlaybackAudio::OnAudioPlaybackPercent(const UAudioComponent* InAudioComponent, const USoundWave* InSoundWave, float Percent)
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
	*VisemeBlends[PreviousVisemeInfo.Code] = FMath::Clamp(1.f - Blend, 0.f, 1.f);
	*VisemeBlends[CurrentVisemeInfo.Code] = FMath::Clamp(Blend, 0.f, 1.f);

	OnVisemeBlendsUpdated.Broadcast(VisemeBlends);
}

void UInworldCharacterPlaybackAudio::OnAudioFinished(UAudioComponent* InAudioComponent)
{
	VisemeBlends = FInworldCharacterVisemeBlends();
	OnVisemeBlendsUpdated.Broadcast(VisemeBlends);
	OnUtteranceStopped.Broadcast();
	UnlockMessageQueue();
}

float* FInworldCharacterVisemeBlends::operator[](const FString& CodeString)
{
	FProperty* CodeProperty = StaticStruct()->FindPropertyByName(FName(CodeString));
	if (CodeProperty)
	{
		return CodeProperty->ContainerPtrToValuePtr<float>(this);
	}
	return &STOP;
}
