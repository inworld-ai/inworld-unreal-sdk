/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "Completion/InworldLLMCompletionTypes.h"

#include "InworldLLMCompleteChatTypes.generated.h"

UENUM(BlueprintType)
enum class EInworldLLMCompleteChatAuthorRole : uint8
{
	UNKNOWN = 0 UMETA(Hidden),
	SYSTEM = 1 UMETA(DisplayName = "System"),
	USER = 2 UMETA(DisplayName = "User"),
	ASSISTANT = 3 UMETA(DisplayName = "Assistant"),
};

USTRUCT(BlueprintType)
struct INWORLDAILLM_API FInworldLLMCompleteChatMessage
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inworld LLM")
	EInworldLLMCompleteChatAuthorRole Role = EInworldLLMCompleteChatAuthorRole::USER;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inworld LLM")
	FString Content;
};

USTRUCT()
struct INWORLDAILLM_API FInworldLLMRequestCompleteChatMessage
{
	GENERATED_BODY()

public:
	UPROPERTY()
	int32 role;

	UPROPERTY()
	FString content;
};

USTRUCT()
struct INWORLDAILLM_API FInworldLLMRequestCompleteChat : public FInworldLLMRequestBase
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<FInworldLLMRequestCompleteChatMessage> messages;
};

USTRUCT()
struct INWORLDAILLM_API FInworldLLMResponseCompleteChatMessage
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FString role;

	UPROPERTY()
	FString content;
};

USTRUCT()
struct INWORLDAILLM_API FInworldLLMResponseCompleteChatChoice
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FInworldLLMResponseCompleteChatMessage message;

	UPROPERTY()
	FString finishReason;
};

USTRUCT()
struct INWORLDAILLM_API FInworldLLMResponseCompleteChatResult : public FInworldLLMResponseBase
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<FInworldLLMResponseCompleteChatChoice> choices;
};

USTRUCT()
struct INWORLDAILLM_API FInworldLLMResponseCompleteChat
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FInworldLLMResponseCompleteChatResult result;
};
