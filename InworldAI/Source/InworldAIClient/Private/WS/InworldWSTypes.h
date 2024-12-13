/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"

#include "InworldWSTypes.generated.h"


USTRUCT()
struct FInworldWSToken
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FString token;

	UPROPERTY()
	FString type;

	UPROPERTY()
	FString expirationTime;

	UPROPERTY()
	FString sessionId;
};

