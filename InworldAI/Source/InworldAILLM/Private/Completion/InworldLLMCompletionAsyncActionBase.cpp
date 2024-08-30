/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "Completion/InworldLLMCompletionAsyncActionBase.h"
#include "HttpModule.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "InworldAILLMModule.h"
#include "InworldAILLMSettings.h"

UInworldLLMCompletionAsyncActionBase::UInworldLLMCompletionAsyncActionBase(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    const UInworldAILLMSettings* LLMSettings = GetDefault<UInworldAILLMSettings>();
    if (LLMSettings)
    {
        ApiKey = LLMSettings->RuntimeApiKey;
        UserId = LLMSettings->UserId;
        Model = LLMSettings->Model;
    }
}

void UInworldLLMCompletionAsyncActionBase::Activate()
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->SetURL("https://api.inworld.ai/llm/v1alpha/completions:" + GetCompletionType());
    HttpRequest->SetVerb("POST");
    HttpRequest->SetHeader("Content-Type", "application/json");
    HttpRequest->SetHeader("Authorization", "Basic " + ApiKey);

    TSharedPtr<FJsonObject> RequestJson = MakeShared<FJsonObject>();
    FString JsonString;
    TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&JsonString);
    if (GetRequestJson(RequestJson) && FJsonSerializer::Serialize(RequestJson.ToSharedRef(), Writer))
    {
        HttpRequest->SetContentAsString(JsonString);

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION > 3
        HttpRequest->OnRequestProgress64().BindUObject(this, &UInworldLLMCompletionAsyncActionBase::HandleOnRequestProgress64);
#else
        HttpRequest->OnRequestProgress().BindUObject(this, &UInworldLLMCompletionAsyncActionBase::HandleOnRequestProgress);
#endif
        HttpRequest->OnProcessRequestComplete().BindUObject(this, &UInworldLLMCompletionAsyncActionBase::HandleOnProcessRequestComplete);

        HttpRequest->ProcessRequest();
    }
    else
    {
        UE_LOG(LogInworldAILLM, Error, TEXT("Invalid Request Format."));
        HandleComplete(false);
    }
}

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION > 3
void UInworldLLMCompletionAsyncActionBase::HandleOnRequestProgress64(FHttpRequestPtr Request, uint64 BytesSent, uint64 BytesReceived)
#else
void UInworldLLMCompletionAsyncActionBase::HandleOnRequestProgress(FHttpRequestPtr Request, int32 BytesSent, int32 BytesReceived)
#endif
{
    FHttpResponsePtr Response = Request->GetResponse();
    if (!Response.IsValid())
    {
        return;
    }

    const int32 ResponseCode = Response->GetResponseCode();
    if (EHttpResponseCodes::IsOk(ResponseCode) || ResponseCode == EHttpResponseCodes::Unknown)
    {
        HandleNextResponseChunk(Response);
    }
}

void UInworldLLMCompletionAsyncActionBase::HandleOnProcessRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
{
    const int32 ResponseCode = Response->GetResponseCode();
    const FString FullResponseString = Response->GetContentAsString();
    if (EHttpResponseCodes::IsOk(ResponseCode))
    {
        HandleNextResponseChunk(Response);
        if (ProcessedContentLength < FullResponseString.Len())
        {
            UE_LOG(LogInworldAILLM, Error, TEXT("Invalid Response Format. json=%s"), *FullResponseString);
            HandleComplete(false);
        }
        else
        {
            HandleComplete(true);
        }
    }
    else
    {
        UE_LOG(LogInworldAILLM, Error, TEXT("Invalid Inworld Studio response. code=%d error=%s"), ResponseCode, *FullResponseString);
        HandleComplete(false);
    }

    SetReadyToDestroy();
}

void UInworldLLMCompletionAsyncActionBase::HandleNextResponseChunk(FHttpResponsePtr Response)
{
    const FString NextResponseString = Response->GetContentAsString().RightChop(ProcessedContentLength);
    TArray<FString> ResponseStringLines;
    NextResponseString.ParseIntoArrayLines(ResponseStringLines);
    for (const FString& ResponseStringLine : ResponseStringLines)
    {
        TSharedPtr<FJsonObject> JsonObject;
        TSharedRef< TJsonReader<> >  Reader = TJsonReaderFactory<>::Create(ResponseStringLine);
        if (FJsonSerializer::Deserialize(Reader, JsonObject) && HandleResponseJson(JsonObject))
        {
            ProcessedContentLength += ResponseStringLine.Len() + 1;
        }
    }
}
