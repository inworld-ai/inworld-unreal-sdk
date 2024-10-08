/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "InworldLLMCompletionAsyncActionBase.h"
#include "Completion/InworldLLMCompleteTextTypes.h"
#include "InworldLLMCompleteTextAsyncAction.generated.h"

UCLASS()
class INWORLDAILLM_API UInworldLLMCompleteTextAsyncAction : public UInworldLLMCompletionAsyncActionBase
{
	GENERATED_BODY()

public:
	/**
	 * Completes the text generation asynchronously.
	 * @param Text The input text for text generation.
	 * @param TextGenerationConfig The configuration for text generation.
	 * @param ApiKeyOverride (Optional) Override for the API key.
	 * @param UserIdOverride (Optional) Override for the user ID.
	 * @param ModelOverride (Optional) Override for the model.
	 * @return A pointer to the async action for completing text generation.
	 */
	UFUNCTION(BlueprintCallable, Category = "LLMService", meta = (AdvancedDisplay = "2", AutoCreateRefTerm = "TextGenerationConfig", BlueprintInternalUseOnly = "true"))
	static UInworldLLMCompleteTextAsyncAction* CompleteText(const FString& Text, const FInworldLLMTextGenerationConfig& TextGenerationConfig, const FString& ApiKeyOverride = "", const FString& UserIdOverride = "", const FString& ModelOverride = "");

protected:
	virtual FString GetCompletionType() const override { return "completeText"; }

	virtual bool GetRequestJson(TSharedPtr<FJsonObject>& RequestJson) const override;
	virtual bool HandleResponseJson(const TSharedPtr<FJsonObject>& ResponseJson) override;

	virtual void HandleComplete(bool bSuccess) override;

private:
	FInworldLLMRequestCompleteText Request;
	FString AccumulatedResponse;
};
