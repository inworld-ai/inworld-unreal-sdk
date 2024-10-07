/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
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
	/**
	 * The key representing the emotional behavior of the character.
	 */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Animation", DisplayName = "Emotion")
	EInworldCharacterEmotionalBehavior Key = EInworldCharacterEmotionalBehavior::NEUTRAL;

	/**
	 * The strength of the character's emotion.
	 */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Animation")
	EInworldCharacterEmotionStrength Strength = EInworldCharacterEmotionStrength::UNSPECIFIED;

	/**
	 * An array of animation montages for the character.
	 */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Animation")
	TArray<UAnimMontage*> Montages;
};

USTRUCT(BlueprintType)
struct INWORLDAIINTEGRATION_API FInworldSemanticGestureTableRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	/**
	 * The key representing the semantics of the animation.
	 */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Animation", DisplayName = "Semantics")
	FString Key;

	/**
	 * The strength of the character's emotion.
	 */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Animation")
	EInworldCharacterEmotionStrength Strength = EInworldCharacterEmotionStrength::UNSPECIFIED;

	/**
	 * An array of animation montages for the character.
	 */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Animation")
	TArray<UAnimMontage*> Montages;
};

UCLASS(meta = (ScriptName = "InworldCharacterAnimationsLib"))
class INWORLDAIINTEGRATION_API UInworldCharacterAnimationsLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Retrieves the animation montage for a specific emotion and emotion strength.
	 *
	 * @param AnimationDT The animation data table to search.
	 * @param Emotion The emotional behavior of the character.
	 * @param EmotionStrength The strength of the character's emotion.
	 * @param UtteranceDuration The duration of the utterance.
	 * @param bAllowTrailingGestures Whether to allow trailing gestures.
	 * @param bFindNeutralGestureIfSearchFailed Whether to find a neutral gesture if the search fails.
	 * @param Montages An array of animation montages.
	 * @return The animation montage for the specified emotion and emotion strength.
	 */
	UFUNCTION(BlueprintPure, Category = "Inworld")
	static UAnimMontage* GetMontageForEmotion(const UDataTable* AnimationDT, EInworldCharacterEmotionalBehavior Emotion, EInworldCharacterEmotionStrength EmotionStrength, float UtteranceDuration, bool bAllowTrailingGestures, bool bFindNeutralGestureIfSearchFailed, UPARAM(ref) TArray<UAnimMontage*>& Montages);

	/**
	 * Retrieves the animation montage for a custom gesture based on semantics.
	 *
	 * @param AnimationDT The animation data table to search.
	 * @param Semantic The semantic representation of the custom gesture.
	 * @param UtteranceDuration The duration of the utterance.
	 * @param bAllowTrailingGestures Whether to allow trailing gestures.
	 * @param bFindNeutralGestureIfSearchFailed Whether to find a neutral gesture if the search fails.
	 * @return The animation montage for the custom gesture.
	 */
	UFUNCTION(BlueprintPure, Category = "Inworld")
	static UAnimMontage* GetMontageForCustomGesture(const UDataTable* AnimationDT, const FString& Semantic, float UtteranceDuration, bool bAllowTrailingGestures, bool bFindNeutralGestureIfSearchFailed);
};
