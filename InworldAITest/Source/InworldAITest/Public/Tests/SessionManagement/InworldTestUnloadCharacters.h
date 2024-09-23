/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "InworldTestFlags.h"
#include "InworldCharacter.h"
#include "TestObjects/InworldTestObjectSession.h"
#include "InworldTestUnloadCharacters.generated.h"

UCLASS()
class UInworldTestObjectUnloadCharacters : public UInworldTestObjectSession
{
	GENERATED_BODY()
public:
	UInworldTestObjectUnloadCharacters();

	UPROPERTY()
	TArray<TObjectPtr<UInworldCharacter>> UnloadCharacters;
};

namespace Inworld
{
	namespace Test
	{
		IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUnloadCharacters, "Inworld.SessionManagement.UnloadCharacters", Flags)
	}
}
