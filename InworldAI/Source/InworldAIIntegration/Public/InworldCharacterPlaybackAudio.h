/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/AudioComponent.h"
#include "InworldCharacterPlayback.h"
#include "InworldIntegrationTypes.h"

#include "InworldCharacterPlaybackAudio.generated.h"

UCLASS(BlueprintType, Blueprintable)
class INWORLDAIINTEGRATION_API UInworldCharacterPlaybackAudio : public UInworldCharacterPlayback
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInworldCharacterUtteranceStarted, float, Duration, FString,  CustomGesture);
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
	FOnInworldCharacterUtteranceStarted OnUtteranceStarted;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInworldCharacterUtteranceStopped);
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
	FOnInworldCharacterUtteranceStopped OnUtteranceStopped;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInworldCharacterUtteranceInterrupted);
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
	FOnInworldCharacterUtteranceInterrupted OnUtteranceInterrupted;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInworldCharacterSilenceStarted, float, Duration);
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
	FOnInworldCharacterSilenceStarted OnSilenceStarted;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInworldCharacterSilenceStopped);
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
	FOnInworldCharacterSilenceStopped OnSilenceStopped;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInworldCharacterVisemeBlendsUpdated, FInworldCharacterVisemeBlends, VisemeBlends);
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
	FOnInworldCharacterVisemeBlendsUpdated OnVisemeBlendsUpdated;

	virtual void BeginPlay_Implementation() override;
	virtual void EndPlay_Implementation() override;

	virtual void OnCharacterUtterance_Implementation(const FCharacterMessageUtterance& Message) override;
	virtual void OnCharacterUtteranceInterrupt_Implementation(const FCharacterMessageUtterance& Message) override;

	virtual void OnCharacterSilence_Implementation(const FCharacterMessageSilence& Message) override;
	virtual void OnCharacterSilenceInterrupt_Implementation(const FCharacterMessageSilence& Message) override;

	void OnSilenceEnd();

	UFUNCTION(BlueprintCallable, Category = "Inworld")
	float GetRemainingTimeForCurrentUtterance() const;

	UFUNCTION(BlueprintCallable, Category = "Inworld")
	const FInworldCharacterVisemeBlends& GetVismeBlends() const { return VisemeBlends; }

protected:
	TWeakObjectPtr<UAudioComponent> AudioComponent;

	UPROPERTY()
	USoundWave* SoundWave;

	float SoundDuration = 0.f;

private:
	void OnAudioPlaybackPercent(const UAudioComponent* InAudioComponent, const USoundWave* InSoundWave, float Percent);
	void OnAudioFinished(UAudioComponent* InAudioComponent);

	FDelegateHandle AudioPlaybackPercentHandle;
	FDelegateHandle AudioFinishedHandle;

	float CurrentAudioPlaybackPercent = 0.f;

	TArray<FCharacterUtteranceVisemeInfo> VisemeInfoPlayback;
	FCharacterUtteranceVisemeInfo CurrentVisemeInfo;
	FCharacterUtteranceVisemeInfo PreviousVisemeInfo;

	FInworldCharacterVisemeBlends VisemeBlends;

	FTimerHandle SilenceTimerHandle;
};

