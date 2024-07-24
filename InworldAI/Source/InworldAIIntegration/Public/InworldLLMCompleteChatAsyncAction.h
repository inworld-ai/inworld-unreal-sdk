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
#include "InworldLLMCompleteChatAsyncAction.generated.h"

UCLASS()
class INWORLDAIINTEGRATION_API UInworldLLMCompleteChatAsyncAction : public UInworldLLMCompletionAsyncActionBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "LLMService")
	static UInworldLLMCompleteChatAsyncAction *CompleteChat(const FString &UserMessage, const FString &SystemMessage, const FString &ApiKey, const FString &ModelName = FString("inworld-dragon"), const FInworldLLMTextGenerationConfig &TextGenerationConfig = FInworldLLMTextGenerationConfig());

	virtual void Activate() override;

private:
	virtual FInworldLLMApiResponse ParseJsonResponse(const FString &JsonString) override;

	FString UserMessage;
	FString SystemMessage;
};