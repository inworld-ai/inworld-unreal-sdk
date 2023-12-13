// Copyright 2023 Theai, Inc. (DBA Inworld) All Rights Reserved.

#pragma once


#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"

#include "InworldEditorUtilityWidget.generated.h"

USTRUCT(BlueprintType)
struct FInworldEditorUtilityWidgetState
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = "Studio")
	int32 WorkspaceIdx = -1;

	UPROPERTY(BlueprintReadWrite, Category = "Studio")
	int32 SceneIdx = -1;

	UPROPERTY(BlueprintReadWrite, Category = "Studio")
	int32 ApiKeyIdx = -1;
};

UCLASS(Abstract, meta = (ShowWorldContextPin), config = InworldAI)
class  UInworldEditorUtilityWidget : public UEditorUtilityWidget
{
	GENERATED_BODY()

public:
	virtual bool Initialize() override;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCosmetic, Category = "User Interface")
	void OnInitializedReal(const FInworldEditorUtilityWidgetState& State);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "User Interface")
	void UpdateState(const FInworldEditorUtilityWidgetState& State);
};
