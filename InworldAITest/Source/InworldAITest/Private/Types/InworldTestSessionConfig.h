/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "InworldTestSessionConfig.generated.h"

USTRUCT()
struct FInworldTestSessionConfig
{
	GENERATED_BODY()

public:
	FInworldTestSessionConfig() = default;
	FInworldTestSessionConfig(const FString& InSceneName)
		: SceneName(InSceneName)
	{}

	UPROPERTY()
	FString SceneName;
};
