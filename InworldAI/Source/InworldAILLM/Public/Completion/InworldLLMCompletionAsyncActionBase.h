/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Completion/InworldLLMCompletionTypes.h"
#include "InworldLLMCompletionAsyncActionBase.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInworldLLMApiDelegate, const FString&, Response);

UCLASS(Abstract)
class INWORLDAILLM_API UInworldLLMCompletionAsyncActionBase : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UInworldLLMCompletionAsyncActionBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void Activate() override;

	UPROPERTY(BlueprintAssignable, Category = "LLMService")
	FInworldLLMApiDelegate OnProgress;

	UPROPERTY(BlueprintAssignable, Category = "LLMService")
	FInworldLLMApiDelegate OnComplete;

	UPROPERTY(BlueprintAssignable, Category = "LLMService")
	FInworldLLMApiDelegate OnFailure;

protected:
	FString ApiKey;
	FString UserId;
	FString Model;

protected:
	virtual FString GetCompletionType() const PURE_VIRTUAL(UInworldLLMCompletionAsyncActionBase::GetCompletionType, return FString{};)

	virtual bool GetRequestJson(TSharedPtr<FJsonObject>&RequestJson) const PURE_VIRTUAL(UInworldLLMCompletionAsyncActionBase::GetRequestJson, return bool{};)
	virtual bool HandleResponseJson(const TSharedPtr<FJsonObject>& ResponseJson) PURE_VIRTUAL(UInworldLLMCompletionAsyncActionBase::HandleResponse, return bool{};)

	virtual void HandleComplete(bool bSuccess) PURE_VIRTUAL(UInworldLLMCompletionAsyncActionBase::HandleComplete)

private:
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION > 3
	void HandleOnRequestProgress64(FHttpRequestPtr Request, uint64 BytesSent, uint64 BytesReceived);
#else
	void HandleOnRequestProgress(FHttpRequestPtr Request, int32 BytesSent, int32 BytesReceived);
#endif
	void HandleOnProcessRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess);
	void HandleNextResponseChunk(FHttpResponsePtr Response);

	uint64 ProcessedContentLength = 0;
};
