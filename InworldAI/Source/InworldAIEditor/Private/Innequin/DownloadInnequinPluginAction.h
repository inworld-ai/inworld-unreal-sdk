/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityLibrary.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "DownloadInnequinPluginAction.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDownloadInnequinOutputPin, bool, bSuccess);

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnDownloadInnequinLog, const FString&, Message);

UCLASS()
class INWORLDAIEDITOR_API UDownloadInnequinPluginAction : public UEditorUtilityBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FDownloadInnequinOutputPin DownloadComplete;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "Flow Control")
	static UDownloadInnequinPluginAction* DownloadInnequinPlugin(FOnDownloadInnequinLog InLogCallback);

	// UBlueprintAsyncActionBase interface
	virtual void Activate() override;
	//~UBlueprintAsyncActionBase interface

private:
	void NotifyLog(const FString& Message);
	void NotifyComplete(bool bSuccess);

	FOnDownloadInnequinLog LogCallback;
};
