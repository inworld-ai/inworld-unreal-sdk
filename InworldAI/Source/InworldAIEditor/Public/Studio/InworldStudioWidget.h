/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once


#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"

#include "InworldStudioWidget.generated.h"

USTRUCT(BlueprintType)
struct FInworldStudioWidgetState
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
class  UInworldStudioWidget : public UEditorUtilityWidget
{
	GENERATED_BODY()

public:
	virtual bool Initialize() override;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCosmetic, Category = "User Interface")
	void OnInitializedReal(const FInworldStudioWidgetState& State);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "User Interface")
	void UpdateState(const FInworldStudioWidgetState& State);
};
