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
#include "InworldEnums.h"
#include "InworldAITestSettings.generated.h"

UCLASS(config = InworldAI)
class INWORLDAITEST_API UInworldAITestSettings : public UObject
{
	GENERATED_BODY()
public:
	UInworldAITestSettings(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(config, EditAnywhere, Category = "Test")
	FString RuntimeApiKey;

	UPROPERTY(config, EditAnywhere, Category = "Test|Session")
	FString Workspace;

	UPROPERTY(config, EditAnywhere, Category = "Test|Session")
	FInworldScene Scene;

	UPROPERTY(config, EditAnywhere, Category = "Test|Session")
	TArray<FString> InitialCharacterNames;

	UPROPERTY(config, EditAnywhere, Category = "Test|Load")
	TArray<FString> CharacterNamesToLoad;

	UPROPERTY(config, EditAnywhere, Category = "Test|Load")
	TArray<FString> CharacterNamesToUnload;
};
