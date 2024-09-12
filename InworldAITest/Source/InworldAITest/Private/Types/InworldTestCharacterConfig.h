/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "InworldTestCharacterConfig.generated.h"

USTRUCT()
struct FInworldTestCharacterConfig
{
	GENERATED_BODY()

public:
	FInworldTestCharacterConfig() = default;
	FInworldTestCharacterConfig(const FString& InCharacterName)
		: CharacterName(InCharacterName)
	{}

	UPROPERTY()
	FString CharacterName;
};
