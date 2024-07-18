/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldLLMCompletionAsyncActionBase.h"

void UInworldLLMCompletionAsyncActionBase::HandleResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
{
}

void UInworldLLMCompletionAsyncActionBase::ProcessStreamedResponse(const FString &ResponseChunk)
{
}

void UInworldLLMCompletionAsyncActionBase::FinishResponse(FString &ResponseChunk)
{
}

FInworldLLMApiResponse UInworldLLMCompletionAsyncActionBase::ParseJsonResponse(const FString &JsonString)
{
	return FInworldLLMApiResponse();
}
