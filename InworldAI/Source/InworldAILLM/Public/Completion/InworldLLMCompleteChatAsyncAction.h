/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "InworldLLMCompletionAsyncActionBase.h"
#include "Completion/InworldLLMCompleteChatTypes.h"
#include "InworldLLMCompleteChatAsyncAction.generated.h"

UCLASS()
class INWORLDAILLM_API UInworldLLMCompleteChatAsyncAction : public UInworldLLMCompletionAsyncActionBase
{
	GENERATED_BODY()

public:
	/**
	 * Complete a chat interaction asynchronously.
	 * @param Messages The array of chat messages to process.
	 * @param TextGenerationConfig The text generation configuration.
	 * @param ApiKeyOverride (Optional) Override for the API key.
	 * @param UserIdOverride (Optional) Override for the user ID.
	 * @param ModelOverride (Optional) Override for the model.
	 * @param ServiceProviderOverride (Optional) Override for service provider.
	 * @param JsonMode (Optional) Coerce model to return valid JSON. Only available for OpenAI models.
	 * @return A pointer to the async action for completing the chat.
	 */
	UFUNCTION(BlueprintCallable, Category = "LLMService", meta = (AdvancedDisplay = "2", AutoCreateRefTerm = "TextGenerationConfig", BlueprintInternalUseOnly = "true"))
	static UInworldLLMCompleteChatAsyncAction* CompleteChat(const TArray<FInworldLLMCompleteChatMessage>& Messages, const FInworldLLMTextGenerationConfig& TextGenerationConfig, const FString& ApiKeyOverride = "", const FString& UserIdOverride = "", const FString& ModelOverride = "", const FString& ServiceProviderOverride = "", bool JsonMode = false);

protected:
	virtual FString GetCompletionType() const override { return "completeChat"; }

	virtual bool GetRequestJson(TSharedPtr<FJsonObject>& RequestJson) const override;
	virtual bool HandleResponseJson(const TSharedPtr<FJsonObject>& ResponseJson) override;

	virtual void HandleComplete(bool bSuccess) override;

private:
	FInworldLLMRequestCompleteChat Request;
	FString AccumulatedResponse;
	bool bJsonMode;
};
