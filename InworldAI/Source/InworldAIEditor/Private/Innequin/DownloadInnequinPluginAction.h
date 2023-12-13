// Copyright 2023 Theai, Inc. (DBA Inworld) All Rights Reserved.

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
