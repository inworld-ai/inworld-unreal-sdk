// Fill out your copyright notice in the Description page of Project Settings.

#include "InworldLLMCompleteChatAsyncAction.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Json.h"
#include "Logging/StructuredLog.h"

FInworldLLMApiResponse UInworldLLMCompleteChatAsyncAction::ParseJsonResponse(const FString &JsonString)
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

                    const TSharedPtr<FJsonObject> *MessageObject;
                    if ((*FirstChoice)->TryGetObjectField(TEXT("message"), MessageObject))
                    {
                        (*MessageObject)->TryGetStringField(TEXT("content"), Response.Content);
                        Response.bSuccess = true;
                    }
                }
            }
        }
    }

    return Response;
}

UInworldLLMCompleteChatAsyncAction *UInworldLLMCompleteChatAsyncAction::CompleteChat(const FString &UserMessage, const FString &SystemMessage, const FString &ApiKey, const FString &ModelName, const FInworldLLMTextGenerationConfig TextGenerationConfig)
{
    UInworldLLMCompleteChatAsyncAction *Action = NewObject<UInworldLLMCompleteChatAsyncAction>();
    Action->UserMessage = UserMessage;
    Action->SystemMessage = SystemMessage;
    Action->ApiKey = ApiKey;
    Action->ModelName = ModelName;
    Action->TextGenerationConfig = TextGenerationConfig;
    return Action;
}

void UInworldLLMCompleteChatAsyncAction::Activate()
{
    // Create the JSON payload
    TSharedPtr<FJsonObject> RootObject = MakeShared<FJsonObject>();

    TSharedPtr<FJsonObject> ServingId = MakeShared<FJsonObject>();
    ServingId->SetStringField("user_id", "user-test");

    TSharedPtr<FJsonObject> ModelId = MakeShared<FJsonObject>();
    ModelId->SetStringField("model", ModelName);
    ServingId->SetObjectField("model_id", ModelId);

    RootObject->SetObjectField("serving_id", ServingId);

    TArray<TSharedPtr<FJsonValue>> Messages;
    TSharedPtr<FJsonObject> SystemPrompt = MakeShared<FJsonObject>();
    SystemPrompt->SetStringField("content", SystemMessage);
    SystemPrompt->SetNumberField("role", 1);
    Messages.Add(MakeShared<FJsonValueObject>(SystemPrompt));

    TSharedPtr<FJsonObject> Message = MakeShared<FJsonObject>();
    Message->SetStringField("content", UserMessage);
    Message->SetNumberField("role", 2);
    Messages.Add(MakeShared<FJsonValueObject>(Message));
    RootObject->SetArrayField("messages", Messages);

    TSharedPtr<FJsonObject> TextGenConfigJsonObject = MakeShared<FJsonObject>();
    TextGenConfigJsonObject->SetNumberField("presence_penalty", TextGenerationConfig.PresencePenalty);
    TextGenConfigJsonObject->SetNumberField("repetition_penalty", TextGenerationConfig.RepetitionPenalty);
    TextGenConfigJsonObject->SetBoolField("stream", TextGenerationConfig.bStream);
    TextGenConfigJsonObject->SetNumberField("max_tokens", TextGenerationConfig.MaxTokens);
    RootObject->SetObjectField("text_generation_config", TextGenConfigJsonObject);

    // Convert JSON to string
    FString JsonPayload;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonPayload);
    FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);

    // Create HTTP request
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->SetURL("https://api.inworld.ai/llm/v1alpha/completions:completeChat");
    HttpRequest->SetVerb("POST");
    HttpRequest->SetHeader("Content-Type", "application/json");

    // Set up Basic Auth
    FString AuthHeader = "Basic " + ApiKey;
    UE_LOG(LogTemp, Log, TEXT("AuthHeader: %s"), *AuthHeader);
    HttpRequest->SetHeader("Authorization", AuthHeader);
    HttpRequest->SetContentAsString(JsonPayload);

    // Set up response handling
    HttpRequest->OnProcessRequestComplete().BindUObject(this, &UInworldLLMCompleteChatAsyncAction::HandleResponse);

    // Initialize streaming variables
    AccumulatedResponse.Empty();
    bIsStreamingComplete = false;

    // Send the request
    HttpRequest->ProcessRequest();
}
