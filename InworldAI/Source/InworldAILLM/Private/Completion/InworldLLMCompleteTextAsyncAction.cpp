/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "Completion/InworldLLMCompleteTextAsyncAction.h"
#include "JsonObjectConverter.h"
#include "Algo/Transform.h"

UInworldLLMCompleteTextAsyncAction* UInworldLLMCompleteTextAsyncAction::CompleteText(const FString& Text, const FInworldLLMTextGenerationConfig& TextGenerationConfig, const FString& ApiKeyOverride, const FString& UserIdOverride, const FString& ModelOverride)
{
    UInworldLLMCompleteTextAsyncAction* Action = NewObject<UInworldLLMCompleteTextAsyncAction>();
    if (!ApiKeyOverride.IsEmpty()) Action->ApiKey = ApiKeyOverride;
    if (!UserIdOverride.IsEmpty()) Action->UserId = UserIdOverride;
    if (!ModelOverride.IsEmpty()) Action->Model = ModelOverride;

    Action->Request.serving_id.user_id = Action->UserId;
    Action->Request.serving_id.model_id.model = Action->Model;

    Action->Request.prompt.text = Text;
    
    Action->Request.text_generation_config = TextGenerationConfig;

    return Action;
}

bool UInworldLLMCompleteTextAsyncAction::GetRequestJson(TSharedPtr<FJsonObject>& RequestJson) const
{
    return FJsonObjectConverter::UStructToJsonObject(FInworldLLMRequestCompleteText::StaticStruct(), &Request, RequestJson.ToSharedRef());
}

bool UInworldLLMCompleteTextAsyncAction::HandleResponseJson(const TSharedPtr<FJsonObject>& ResponseJson)
{
    FInworldLLMResponseCompleteText Response;
    if (!FJsonObjectConverter::JsonObjectToUStruct(ResponseJson.ToSharedRef(), &Response) || Response.result.choices.IsEmpty())
    {
        return false;
    }

    const FString& NextChunk = Response.result.choices[0].text;
    AccumulatedResponse += NextChunk;
    OnProgress.Broadcast(NextChunk);

    return true;
}

void UInworldLLMCompleteTextAsyncAction::HandleComplete(bool bSuccess)
{
    if (bSuccess)
    {
        OnComplete.Broadcast(AccumulatedResponse);
    }
    else
    {
        OnFailure.Broadcast({});
    }
}
