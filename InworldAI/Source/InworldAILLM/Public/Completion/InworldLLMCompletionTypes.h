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
	UPROPERTY()
	FString model;
};

USTRUCT()
struct INWORLDAILLM_API FInworldLLMRequestServingId
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FString user_id;

	UPROPERTY()
	FInworldLLMRequestModel model_id;
};

USTRUCT(BlueprintType)
struct INWORLDAILLM_API FInworldLLMTextGenerationConfig
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "LLMService", meta = (DisplayName = "Max Tokens"))
	int32 max_tokens = 150;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "LLMService", meta = (DisplayName = "Temperature"))
	float temperature = 0.5f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "LLMService", meta = (DisplayName = "Presence Penalty"))
	float presence_penalty = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "LLMService", meta = (DisplayName = "Repetition Penalty"))
	float repetition_penalty = 1.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "LLMService", meta = (DisplayName = "Frequency Penalty"))
	float frequency_penalty = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "LLMService", meta = (DisplayName = "Top P"))
	float top_p = 1.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "LLMService", meta = (DisplayName = "Stream"))
	bool stream = false;
};

USTRUCT()
struct INWORLDAILLM_API FInworldLLMRequestBase
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FInworldLLMRequestServingId serving_id;

	UPROPERTY()
	FInworldLLMTextGenerationConfig text_generation_config;
};

USTRUCT()
struct INWORLDAILLM_API FInworldLLMResponseUsage
{
	GENERATED_BODY()

public:
	UPROPERTY()
	int32 completionTokens;

	UPROPERTY()
	int32 promptTokens;
};

USTRUCT(BlueprintType)
struct INWORLDAILLM_API FInworldLLMResponseBase
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FString id;

	UPROPERTY()
	FString createTime;

	UPROPERTY()
	FString model;

	UPROPERTY()
	FInworldLLMResponseUsage usage;
};
