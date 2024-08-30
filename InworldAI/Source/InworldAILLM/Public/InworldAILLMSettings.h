/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "InworldAILLMSettings.generated.h"

UCLASS(config=InworldAI)
class INWORLDAILLM_API UInworldAILLMSettings : public UObject
{
	GENERATED_BODY()

public:
	UInworldAILLMSettings(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(config, EditAnywhere, Category = "Inworld")
	FString RuntimeApiKey;

	UPROPERTY(config, EditAnywhere, Category = "Inworld")
	FString UserId;

	UPROPERTY(config, EditAnywhere, Category = "Inworld")
	FString Model;
};
