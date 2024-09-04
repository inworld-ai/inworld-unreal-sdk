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
	UFUNCTION(BlueprintCallable, Category = "LLMService", meta = (AdvancedDisplay = "2", AutoCreateRefTerm = "TextGenerationConfig", BlueprintInternalUseOnly = "true"))
	static UInworldLLMCompleteChatAsyncAction* CompleteChat(const TArray<FInworldLLMCompleteChatMessage>& Messages, const FInworldLLMTextGenerationConfig& TextGenerationConfig, const FString& ApiKeyOverride = "", const FString& UserIdOverride = "", const FString& ModelOverride = "");

protected:
	virtual FString GetCompletionType() const override { return "completeChat"; }

	virtual bool GetRequestJson(TSharedPtr<FJsonObject>& RequestJson) const override;
	virtual bool HandleResponseJson(const TSharedPtr<FJsonObject>& ResponseJson) override;

	virtual void HandleComplete(bool bSuccess) override;

private:
	FInworldLLMRequestCompleteChat Request;
	FString AccumulatedResponse;
};
