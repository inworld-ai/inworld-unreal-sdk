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
#include "InworldTestObject.generated.h"

USTRUCT()
struct FInworldTestObjectConfig
{
	GENERATED_BODY()
};

UCLASS()
class UInworldTestObject : public UObject
{
	GENERATED_BODY()

public:
	UInworldTestObject() = default;
	UInworldTestObject(FInworldTestObjectConfig Config)
	{
		if (!FParse::Value(FCommandLine::Get(), TEXT("InworldTestRuntimeApiKey="), RuntimeAuth.Base64Signature))
		{
			const UInworldAITestSettings* InworldAITestSettings = GetDefault<UInworldAITestSettings>();
			RuntimeAuth.Base64Signature = InworldAITestSettings->RuntimeApiKey;
		}
	}

	UPROPERTY()
	FInworldAuth RuntimeAuth;
};
