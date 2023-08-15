/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"

#include "InworldEnums.h"
#include <Engine/DataTable.h>
#include <Kismet/BlueprintFunctionLibrary.h>

#include "InworldCharacterAnimations.generated.h"

USTRUCT(BlueprintType)
struct INWORLDAIINTEGRATION_API FInworldAnimationTableRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Animation", DisplayName = "Emotion")
	EInworldCharacterEmotionalBehavior Key = EInworldCharacterEmotionalBehavior::NEUTRAL;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Animation")
	EInworldCharacterEmotionStrength Strength = EInworldCharacterEmotionStrength::UNSPECIFIED;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Animation")
	TArray<UAnimMontage*> Montages;
};

USTRUCT(BlueprintType)
struct INWORLDAIINTEGRATION_API FInworldSemanticGestureTableRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Animation", DisplayName = "Semantics")
	FString Key;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Animation")
	EInworldCharacterEmotionStrength Strength = EInworldCharacterEmotionStrength::UNSPECIFIED;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Animation")
	TArray<UAnimMontage*> Montages;
};

UCLASS(meta = (ScriptName = "InworldCharacterAnimationsLib"))
class INWORLDAIINTEGRATION_API UInworldCharacterAnimationsLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Inworld")
	static UAnimMontage* GetMontageForEmotion(const UDataTable* AnimationDT, EInworldCharacterEmotionalBehavior Emotion, EInworldCharacterEmotionStrength EmotionStrength, float UtteranceDuration, bool bAllowTrailingGestures, bool bFindNeutralGestureIfSearchFailed, UPARAM(ref) TArray<UAnimMontage*>& Montages);

	UFUNCTION(BlueprintPure, Category = "Inworld")
	static UAnimMontage* GetMontageForCustomGesture(const UDataTable* AnimationDT, const FString& Semantic, float UtteranceDuration, bool bAllowTrailingGestures, bool bFindNeutralGestureIfSearchFailed);
};