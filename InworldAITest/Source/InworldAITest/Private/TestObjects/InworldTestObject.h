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


UCLASS()
class UInworldTestObject : public UObject
{
	GENERATED_BODY()

public:
	UInworldTestObject()
	{
		const UInworldAITestSettings* InworldAITestSettings = GetDefault<UInworldAITestSettings>();
		if (!FParse::Value(FCommandLine::Get(), TEXT("InworldTestRuntimeApiKey="), RuntimeAuth.Base64Signature))
		{
			RuntimeAuth.Base64Signature = InworldAITestSettings->RuntimeApiKey;
		}
	}

	UPROPERTY()
	FInworldAuth RuntimeAuth;
};
