/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "InworldTypes.h"
#include "InworldAIClientSettings.generated.h"

UCLASS(config=InworldAI)
class INWORLDAICLIENT_API UInworldAIClientSettings : public UObject
{
	GENERATED_BODY()
public:
	UInworldAIClientSettings(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(config, EditAnywhere, Category = "Inworld")
	FString Workspace;

	UPROPERTY(config, EditAnywhere, Category = "Inworld")
	FInworldAuth Auth;

	UPROPERTY(config, EditAnywhere, Category = "Inworld", meta = (ShowOnlyInnerProperties))
	FInworldEnvironment Environment;
};