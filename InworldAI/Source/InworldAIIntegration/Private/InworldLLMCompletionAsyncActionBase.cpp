// Fill out your copyright notice in the Description page of Project Settings.


#include "InworldLLMCompletionAsyncActionBase.h"

void UInworldLLMCompletionAsyncActionBase::HandleResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
{
}

void UInworldLLMCompletionAsyncActionBase::ProcessStreamedResponse(const FString& ResponseChunk)
{
}

void UInworldLLMCompletionAsyncActionBase::FinishResponse(FString& ResponseChunk)
{
}

FInworldLLMApiResponse UInworldLLMCompletionAsyncActionBase::ParseJsonResponse(const FString& JsonString)
{
	return FInworldLLMApiResponse();
}
