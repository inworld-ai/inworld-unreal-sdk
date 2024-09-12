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
#include "InworldTestFlags.h"
#include "TestObjects/InworldTestObjectSession.h"
#include "InworldTestStartSessionByScene.generated.h"

UCLASS()
class UInworldTestObjectStartSessionByScene : public UInworldTestObjectSession
{
	GENERATED_BODY()

public:
	UInworldTestObjectStartSessionByScene()
		: UInworldTestObjectSession(
			FInworldTestSessionConfig{ TEXT("workspaces/sdk_test_automation/scenes/full_scene") },
			TArray<FInworldTestCharacterConfig>{
				FInworldTestCharacterConfig{ TEXT("character_one") },
				FInworldTestCharacterConfig{ TEXT("character_two") },
				FInworldTestCharacterConfig{ TEXT("character_three") },
			}
	)
	{}
};

namespace Inworld
{
	namespace Test
	{
		IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStartSessionByScene, "Inworld.SessionManagement.StartSessionByScene", Flags)
		bool FStartSessionByScene::RunTest(const FString& Parameters)
		{
			TInworldTestObjectSessionScoped<UInworldTestObjectStartSessionByScene> TestObject(this);

			return true;
		}
	}
}
