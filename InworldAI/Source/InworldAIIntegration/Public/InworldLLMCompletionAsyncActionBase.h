/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Interfaces/IHttpRequest.h"
#include "InworldLLMCompletionAsyncActionBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FInworldLLMApiDelegate, const FString &, PartialResponse, const FString &, FullResponse);

USTRUCT(BlueprintType)
struct FInworldLLMTextGenerationConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LLMService")
	float PresencePenalty = 0.8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LLMService")
	float RepetitionPenalty = 1.2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LLMService")
	bool bStream = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LLMService")
	int32 MaxTokens = 150;
};

USTRUCT()
struct FInworldLLMApiResponse
{
	GENERATED_BODY()

	FString FinishReason;
	FString Content;
	bool bSuccess;

	FInworldLLMApiResponse() : bSuccess(false) {}
};

UCLASS()
class INWORLDAIINTEGRATION_API UInworldLLMCompletionAsyncActionBase : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "LLMService")
	FInworldLLMApiDelegate OnProgress;

	UPROPERTY(BlueprintAssignable, Category = "LLMService")
	FInworldLLMApiDelegate OnComplete;

	UPROPERTY(BlueprintAssignable, Category = "LLMService")
	FInworldLLMApiDelegate OnFailure;

protected:
	virtual void HandleResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess);
	virtual void ProcessStreamedResponse(const FString &ResponseChunk);
	virtual void FinishResponse(FString &ResponseChunk);

	virtual FInworldLLMApiResponse ParseJsonResponse(const FString &JsonString);

	FString ApiKey;
	FString ModelName;
	FString AccumulatedResponse;
	bool bIsStreamingComplete;
	FInworldLLMTextGenerationConfig TextGenerationConfig;
};
