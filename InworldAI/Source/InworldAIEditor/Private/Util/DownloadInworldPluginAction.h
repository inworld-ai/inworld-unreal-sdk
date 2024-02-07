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
#include "DownloadInworldPluginAction.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDownloadInworldOutputPin, bool, bSuccess);

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnDownloadInworldLog, const FString&, Message);

UCLASS()
class INWORLDAIEDITOR_API UDownloadInworldPluginAction : public UEditorUtilityBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	UPROPERTY()
	FString ZipURL;

	UPROPERTY(BlueprintAssignable)
	FDownloadInworldOutputPin DownloadComplete;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "Flow Control")
	static UDownloadInworldPluginAction* DownloadInworldPlugin(const FString& InZipURL, FOnDownloadInworldLog InLogCallback);

	// UBlueprintAsyncActionBase interface
	virtual void Activate() override;
	//~UBlueprintAsyncActionBase interface

private:
	void NotifyLog(const FString& Message);
	void NotifyComplete(bool bSuccess);

	FOnDownloadInworldLog LogCallback;
};
