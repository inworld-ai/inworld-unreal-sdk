/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/InworldTestFlags.h"
#include "InworldTestTwoPlusTwo.generated.h"

USTRUCT()
struct FInworldTestTwoPlusTwo
{
	GENERATED_BODY()
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInworldTwoPlusTwo, "Inworld.TwoPlusTwo", Inworld::Test::Flags)
bool FInworldTwoPlusTwo::RunTest(const FString& Parameters)
{
	return TestEqual("Testing that 2+2 == 4", 2 + 2, 4);
}
