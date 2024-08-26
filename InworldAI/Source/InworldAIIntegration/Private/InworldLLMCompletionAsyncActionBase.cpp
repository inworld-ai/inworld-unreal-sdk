/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldLLMCompletionAsyncActionBase.h"
#include "Interfaces/IHttpResponse.h"

void UInworldLLMCompletionAsyncActionBase::HandleResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
{
	if (bSuccess && Response.IsValid())
	{
		bIsStreamingComplete = true;
		FString ResponseChunk = Response->GetContentAsString();
		FinishResponse(ResponseChunk);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Request failed"));
		OnFailure.Broadcast(FString(), FString());
	}

	SetReadyToDestroy();
}

void UInworldLLMCompletionAsyncActionBase::ProcessStreamedResponse(const FString &ResponseChunk)
{
	// Split the accumulated response into lines
	TArray<FString> Lines;
	ResponseChunk.ParseIntoArrayLines(Lines);

	for (const FString &Line : Lines)
	{
		if (Line.IsEmpty())
		{
			continue;
		}

		FInworldLLMApiResponse Response = ParseJsonResponse(Line);

		if (Response.bSuccess)
		{
			OnProgress.Broadcast(Response.Content, FString());
			AccumulatedResponse += Response.Content;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to parse JSON %s"), *Line);
			OnFailure.Broadcast(FString(), FString());
		}
	}
}

void UInworldLLMCompletionAsyncActionBase::FinishResponse(FString &ResponseChunk)
{
	AccumulatedResponse.Empty();
	ProcessStreamedResponse(ResponseChunk);
	OnComplete.Broadcast(FString(), AccumulatedResponse);
}

FInworldLLMApiResponse UInworldLLMCompletionAsyncActionBase::ParseJsonResponse(const FString &JsonString)
{
	return FInworldLLMApiResponse();
}
