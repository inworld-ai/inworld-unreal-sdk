/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */
#include "InworldCharacterAnimations.h"
#include "Animation/AnimMontage.h"

#include "InworldAIIntegrationModule.h"

template<class TKey, class TDataTable>
UAnimMontage* GetMontageByKey(TKey Key, float UtteranceDuration, const UDataTable* DataTable, bool bAllowTrailingGestures, EInworldCharacterEmotionStrength EmotionStrength, TArray<UAnimMontage*>& Montages)
{
	if (!ensure(DataTable))
	{
		UE_LOG(LogInworldAIIntegration, Error, TEXT("Setup animation data table!"));
		return nullptr;
	}

	if (Montages.Num() == 0)
	{
		DataTable->ForeachRow<TDataTable>(TEXT("InworldAnim"),
			[&Montages, Key, EmotionStrength, DataTable](const FName& Name, const TDataTable& Row)
			{
				if (Key == Row.Key &&
					ensureMsgf(Row.Montages.Num() > 0, TEXT("You must add montages to Data Table, %s:%s"), *DataTable->GetName(), *Name.ToString()) &&
					EmotionStrength == Row.Strength)
				{
					Montages = Row.Montages;
				}
			});
	}

	if (Montages.Num() == 0)
	{
		return nullptr;
	}

	int32 Idx = -1;
	float DurationDif = TNumericLimits<float>::Max();
	for (int32 i = 0; i < Montages.Num(); i++)
	{
		auto* Montage = Montages[i];
		const float MontageDuration = Montage->GetPlayLength();
		float Dif = UtteranceDuration - MontageDuration;
		if (bAllowTrailingGestures)
		{
			Dif = FMath::Abs(Dif);
		}

		if (Dif >= 0.f && Dif < DurationDif)
		{
			Idx = i;
			DurationDif = Dif;
		}
	}

	if (Idx == -1)
	{
		return nullptr;
	}

	auto* Montage = Montages[Idx];
	Montages.RemoveAt(Idx);

	return Montage;
}

UAnimMontage* UInworldCharacterAnimationsLib::GetMontageForEmotion(const UDataTable* AnimationDT, EInworldCharacterEmotionalBehavior Emotion, EInworldCharacterEmotionStrength EmotionStrength, float UtteranceDuration, bool bAllowTrailingGestures, bool bFindNeutralGestureIfSearchFailed, UPARAM(ref) TArray<UAnimMontage*>& Montages)
{
	// find montage by emotional state and strength
	auto* Montage = GetMontageByKey<EInworldCharacterEmotionalBehavior, FInworldAnimationTableRow>(Emotion, UtteranceDuration, AnimationDT, bAllowTrailingGestures, EmotionStrength, Montages);
	if (Montage)
	{
		return Montage;
	}

	// find montage by emotional state only(in unspecified strength data table row)
	if (EmotionStrength != EInworldCharacterEmotionStrength::UNSPECIFIED)
	{
		Montage = GetMontageByKey<EInworldCharacterEmotionalBehavior, FInworldAnimationTableRow>(Emotion, UtteranceDuration, AnimationDT, bAllowTrailingGestures, EInworldCharacterEmotionStrength::UNSPECIFIED, Montages);
		if (Montage)
		{
			return Montage;
		}
	}

	// find montage for neutral emotional state
	if (bFindNeutralGestureIfSearchFailed && Emotion != EInworldCharacterEmotionalBehavior::NEUTRAL)
	{
		return GetMontageByKey<EInworldCharacterEmotionalBehavior, FInworldAnimationTableRow>(EInworldCharacterEmotionalBehavior::NEUTRAL, UtteranceDuration, AnimationDT, bAllowTrailingGestures, EInworldCharacterEmotionStrength::UNSPECIFIED, Montages);
	}

	return nullptr;
}

UAnimMontage* UInworldCharacterAnimationsLib::GetMontageForCustomGesture(const UDataTable* AnimationDT, const FString& Semantic, float UtteranceDuration, bool bAllowTrailingGestures, bool bFindNeutralGestureIfSearchFailed)
{
	TArray<UAnimMontage*> Montages;
	return GetMontageByKey<FString, FInworldSemanticGestureTableRow>(Semantic, UtteranceDuration, AnimationDT, bAllowTrailingGestures, EInworldCharacterEmotionStrength::UNSPECIFIED, Montages);
}
