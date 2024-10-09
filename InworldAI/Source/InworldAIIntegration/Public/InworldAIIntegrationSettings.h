/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "InworldAIIntegrationSettings.generated.h"

UCLASS(config=InworldAI)
class INWORLDAIINTEGRATION_API UInworldAIIntegrationSettings : public UObject
{
	GENERATED_BODY()
public:
	UInworldAIIntegrationSettings(const FObjectInitializer& ObjectInitializer);
	
	/**
	 * The URL for Inworld Studio.
	 */
	UPROPERTY(config, EditAnywhere, Category = "Inworld")
	FString StudioApiUrl;

	/**
	 * Studio API key for authenticating with Inworld AI.
	 */
	UPROPERTY(config, EditAnywhere, Category = "Inworld")
	FString StudioApiKey;
};
