/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "Completion/InworldLLMCompleteChatAsyncAction.h"
#include "JsonObjectConverter.h"
#include "Algo/Transform.h"

UInworldLLMCompleteChatAsyncAction* UInworldLLMCompleteChatAsyncAction::CompleteChat(const TArray<FInworldLLMCompleteChatMessage>& Messages, const FInworldLLMTextGenerationConfig& TextGenerationConfig, const FString& ApiKeyOverride, const FString& UserIdOverride, const FString& ModelOverride)
{
    UInworldLLMCompleteChatAsyncAction* Action = NewObject<UInworldLLMCompleteChatAsyncAction>();
    if (!ApiKeyOverride.IsEmpty()) Action->ApiKey = ApiKeyOverride;
    if (!UserIdOverride.IsEmpty()) Action->UserId = UserIdOverride;
    if (!ModelOverride.IsEmpty()) Action->Model = ModelOverride;

    Action->Request.serving_id.user_id = Action->UserId;
    Action->Request.serving_id.model_id.model = Action->Model;

    Action->Request.messages.Reserve(Messages.Num());
    Algo::Transform(Messages, Action->Request.messages, [](const FInworldLLMCompleteChatMessage& Message) { return FInworldLLMRequestCompleteChatMessage{(int32)Message.Role, Message.Content}; });

    Action->Request.text_generation_config = TextGenerationConfig;

    return Action;
}

bool UInworldLLMCompleteChatAsyncAction::GetRequestJson(TSharedPtr<FJsonObject>& RequestJson) const
{
    return FJsonObjectConverter::UStructToJsonObject(FInworldLLMRequestCompleteChat::StaticStruct(), &Request, RequestJson.ToSharedRef());
}

bool UInworldLLMCompleteChatAsyncAction::HandleResponseJson(const TSharedPtr<FJsonObject>& ResponseJson)
{
    FInworldLLMResponseCompleteChat Response;
    if(!FJsonObjectConverter::JsonObjectToUStruct(ResponseJson.ToSharedRef(), &Response) || Response.result.choices.IsEmpty())
    {
        return false;
    }

    const FString& NextChunk = Response.result.choices[0].message.content;
    AccumulatedResponse += NextChunk;
    OnProgress.Broadcast(NextChunk);

    return true;
}

void UInworldLLMCompleteChatAsyncAction::HandleComplete(bool bSuccess)
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
