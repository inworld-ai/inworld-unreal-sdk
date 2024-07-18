// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InworldLLMCompletionAsyncActionBase.h"
#include "Interfaces/IHttpRequest.h"
#include "InworldLLMCompleteChatAsyncAction.generated.h"

UCLASS()
class INWORLDDEMO_API UInworldLLMCompleteChatAsyncAction : public UInworldLLMCompletionAsyncActionBase 
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "LLMService")
	static UInworldLLMCompleteChatAsyncAction* CompleteChat(const FString& UserMessage, const FString& SystemMessage, const FString& ApiKey, const FString& ModelName = FString("inworld-dragon"));

	virtual void Activate() override;

private:
	virtual void HandleResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess) override;
	virtual void ProcessStreamedResponse(const FString& ResponseChunk) override;
	virtual void FinishResponse(FString& ResponseChunk) override;
	virtual FInworldLLMApiResponse ParseJsonResponse(const FString& JsonString) override;

	FString UserMessage;
	FString SystemMessage;
};