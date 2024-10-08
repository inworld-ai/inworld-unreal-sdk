/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "Completion/InworldLLMCompletionTypes.h"

#include "InworldLLMCompleteTextTypes.generated.h"

USTRUCT()
struct INWORLDAILLM_API FInworldLLMRequestCompleteTextPrompt
{
	GENERATED_BODY()

public:
	/**
	 * The text prompt for completing text.
	 */
	UPROPERTY()
	FString text;
};

USTRUCT()
struct INWORLDAILLM_API FInworldLLMRequestCompleteText : public FInworldLLMRequestBase
{
	GENERATED_BODY()

public:
	/**
	 * The prompt for completing text.
	 */
	UPROPERTY()
	FInworldLLMRequestCompleteTextPrompt prompt;
};

USTRUCT()
struct INWORLDAILLM_API FInworldLLMResponseCompleteTextChoice
{
	GENERATED_BODY()

public:
	/**
	 * The text choice for completing text.
	 */
	UPROPERTY()
	FString text;

	/**
	 * The reason for finishing the text completion.
	 */
	UPROPERTY()
	FString finishReason;
};

USTRUCT()
struct INWORLDAILLM_API FInworldLLMResponseCompleteTextResult : public FInworldLLMResponseBase
{
	GENERATED_BODY()

public:
	/**
	 * An array of text choices for completing the text.
	 */
	UPROPERTY()
	TArray<FInworldLLMResponseCompleteTextChoice> choices;
};

USTRUCT()
struct INWORLDAILLM_API FInworldLLMResponseCompleteText
{
	GENERATED_BODY()

public:
	/**
	 * The result of completing the text.
	 */
	UPROPERTY()
	FInworldLLMResponseCompleteTextResult result;
};
