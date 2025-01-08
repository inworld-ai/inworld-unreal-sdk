/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"

#include "InworldLLMCompletionTypes.generated.h"

USTRUCT()
struct INWORLDAILLM_API FInworldLLMRequestModel
{
	GENERATED_BODY()

public:
	/**
	 * The model used in the request.
	 */
	UPROPERTY()
	FString model;

	UPROPERTY()
	FString service_provider;
};

USTRUCT()
struct INWORLDAILLM_API FInworldLLMRequestServingId
{
	GENERATED_BODY()

public:
	/**
	 * The user ID associated with the request.
	 */
	UPROPERTY()
	FString user_id;

	/**
	 * The model ID associated with the request.
	 */
	UPROPERTY()
	FInworldLLMRequestModel model_id;
};

USTRUCT(BlueprintType)
struct INWORLDAILLM_API FInworldLLMTextGenerationConfig
{
	GENERATED_BODY()

public:
	/**
	 * Maximum number of tokens for text generation.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "LLMService", meta = (DisplayName = "Max Tokens"))
	int32 max_tokens = 150;

	/**
	 * Temperature parameter for text generation.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "LLMService", meta = (DisplayName = "Temperature"))
	float temperature = 0.5f;

	/**
	 * Presence penalty for text generation.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "LLMService", meta = (DisplayName = "Presence Penalty"))
	float presence_penalty = 0.f;

	/**
	 * Repetition penalty for text generation.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "LLMService", meta = (DisplayName = "Repetition Penalty"))
	float repetition_penalty = 1.f;

	/**
	 * Frequency penalty for text generation.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "LLMService", meta = (DisplayName = "Frequency Penalty"))
	float frequency_penalty = 0.f;

	/**
	 * Top P parameter for text generation.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "LLMService", meta = (DisplayName = "Top P"))
	float top_p = 1.f;

	/**
	 * Flag indicating whether to stream the text generation.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "LLMService", meta = (DisplayName = "Stream"))
	bool stream = false;
};

USTRUCT()
struct INWORLDAILLM_API FInworldLLMRequestBase
{
	GENERATED_BODY()

public:
	/**
	 * Serving ID for the request.
	 */
	UPROPERTY()
	FInworldLLMRequestServingId serving_id;

	/**
	 * Text generation configuration for the request.
	 */
	UPROPERTY()
	FInworldLLMTextGenerationConfig text_generation_config;

	UPROPERTY()
	FString response_format = "";
};

USTRUCT()
struct INWORLDAILLM_API FInworldLLMResponseUsage
{
	GENERATED_BODY()

public:
	/**
	 * Number of completion tokens in the response.
	 */
	UPROPERTY()
	int32 completionTokens = 0;

	/**
	 * Number of prompt tokens in the response.
	 */
	UPROPERTY()
	int32 promptTokens = 0;
};

USTRUCT(BlueprintType)
struct INWORLDAILLM_API FInworldLLMResponseBase
{
	GENERATED_BODY()

public:
	/**
	 * Unique ID for the response.
	 */
	UPROPERTY()
	FString id;

	/**
	 * Creation time of the response.
	 */
	UPROPERTY()
	FString createTime;

	/**
	 * Model used for generating the response.
	 */
	UPROPERTY()
	FString model;

	/**
	 * Usage information in the response.
	 */
	UPROPERTY()
	FInworldLLMResponseUsage usage;
};
