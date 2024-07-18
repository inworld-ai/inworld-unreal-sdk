/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldLLMCompleteTextAsyncAction.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Json.h"
#include "Logging/StructuredLog.h"

FInworldLLMApiResponse UInworldLLMCompleteTextAsyncAction::ParseJsonResponse(const FString &JsonString)
{
    FInworldLLMApiResponse Response;
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

    if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
    {
        const TSharedPtr<FJsonObject> *ResultObject;
        if (JsonObject->TryGetObjectField(TEXT("result"), ResultObject))
        {
            const TArray<TSharedPtr<FJsonValue>> *ChoicesArray;
            if ((*ResultObject)->TryGetArrayField(TEXT("choices"), ChoicesArray) && ChoicesArray->Num() > 0)
            {
                const TSharedPtr<FJsonObject> *FirstChoice;
                if ((*ChoicesArray)[0]->TryGetObject(FirstChoice))
                {
                    (*FirstChoice)->TryGetStringField(TEXT("finishReason"), Response.FinishReason);
                    (*FirstChoice)->TryGetStringField(TEXT("text"), Response.Content);
                    Response.bSuccess = true;
                }
            }
        }
    }

    return Response;
}

UInworldLLMCompleteTextAsyncAction *UInworldLLMCompleteTextAsyncAction::CompleteText(const FString &UserMessage, const FString &SystemMessage, const FString &ApiKey, const FString &ModelName)
{
    UInworldLLMCompleteTextAsyncAction *Action = NewObject<UInworldLLMCompleteTextAsyncAction>();
    Action->UserMessage = UserMessage;
    Action->ApiKey = ApiKey;
    Action->ModelName = ModelName;
    return Action;
}

void UInworldLLMCompleteTextAsyncAction::Activate()
{
    // Create the JSON payload
    TSharedPtr<FJsonObject> RootObject = MakeShared<FJsonObject>();

    TSharedPtr<FJsonObject> ServingId = MakeShared<FJsonObject>();
    ServingId->SetStringField("user_id", "user-test");

    TSharedPtr<FJsonObject> ModelId = MakeShared<FJsonObject>();
    ModelId->SetStringField("model", ModelName);
    ServingId->SetObjectField("model_id", ModelId);

    RootObject->SetObjectField("serving_id", ServingId);

    TSharedPtr<FJsonObject> Prompt = MakeShared<FJsonObject>();
    Prompt->SetStringField("text", UserMessage);
    RootObject->SetObjectField("prompt", Prompt);

    TSharedPtr<FJsonObject> TextGenerationConfig = MakeShared<FJsonObject>();
    TextGenerationConfig->SetNumberField("presence_penalty", 0.8);
    TextGenerationConfig->SetNumberField("repetition_penalty", 1.2);
    TextGenerationConfig->SetBoolField("stream", false);
    TextGenerationConfig->SetNumberField("max_tokens", 150);
    RootObject->SetObjectField("text_generation_config", TextGenerationConfig);

    // Convert JSON to string
    FString JsonPayload;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonPayload);
    FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);

    // Create HTTP request
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->SetURL("https://api.inworld.ai/llm/v1alpha/completions:completeText");
    HttpRequest->SetVerb("POST");
    HttpRequest->SetHeader("Content-Type", "application/json");

    // Set up Basic Auth
    FString AuthHeader = "Basic " + ApiKey;
    UE_LOG(LogTemp, Log, TEXT("AuthHeader: %s"), *AuthHeader);
    HttpRequest->SetHeader("Authorization", AuthHeader);
    HttpRequest->SetContentAsString(JsonPayload);

    // Set up response handling
    HttpRequest->OnProcessRequestComplete().BindUObject(this, &UInworldLLMCompleteTextAsyncAction::HandleResponse);

    // Initialize streaming variables
    AccumulatedResponse.Empty();
    bIsStreamingComplete = false;

    // Send the request
    HttpRequest->ProcessRequest();
}

void UInworldLLMCompleteTextAsyncAction::HandleResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
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

void UInworldLLMCompleteTextAsyncAction::ProcessStreamedResponse(const FString &ResponseChunk)
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

void UInworldLLMCompleteTextAsyncAction::FinishResponse(FString &ResponseChunk)
{
    AccumulatedResponse.Empty();
    ProcessStreamedResponse(ResponseChunk);
    OnComplete.Broadcast(FString(), AccumulatedResponse);
}
