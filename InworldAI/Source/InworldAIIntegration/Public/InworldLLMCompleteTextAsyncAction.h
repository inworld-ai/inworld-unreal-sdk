/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "InworldLLMCompletionAsyncActionBase.h"
#include "Interfaces/IHttpRequest.h"
#include "InworldLLMCompleteTextAsyncAction.generated.h"

/**
 *
 */
UCLASS()
class INWORLDDEMO_API UInworldLLMCompleteTextAsyncAction : public UInworldLLMCompletionAsyncActionBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "LLMService")
	static UInworldLLMCompleteTextAsyncAction *CompleteText(const FString &UserMessage, const FString &SystemMessage, const FString &ApiKey, const FString &ModelName = FString("inworld-dragon"));

	virtual void Activate() override;

private:
	virtual void HandleResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess) override;
	virtual void ProcessStreamedResponse(const FString &ResponseChunk) override;
	virtual void FinishResponse(FString &ResponseChunk) override;
	virtual FInworldLLMApiResponse ParseJsonResponse(const FString &JsonString) override;

	FString UserMessage;
};