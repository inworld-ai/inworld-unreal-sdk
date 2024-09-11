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
#include "Tests/InworldAITestFlags.h"
#include "Tests/InworldAITestHelpers.h"
#include "Tests/InworldAITestUtil.h"
#include "InworldAITestStartSessionByScene.generated.h"

USTRUCT()
struct FInworldAITestStartSessionByScene
{
	GENERATED_BODY()
};

namespace Inworld
{
	namespace Test
	{
		IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInworldTestStartSessionByScene, "Inworld.SessionManagement.StartSessionByScene", Flags)
		bool FInworldTestStartSessionByScene::RunTest(const FString& Parameters)
		{
			const FString Scene{ TEXT("workspaces/sdk_test_automation/scenes/full_scene") };

			FInworldAuth Auth = GetAuth();

			UInworldSession* Session = NewObject<UInworldSession>();
			ADD_LATENT_AUTOMATION_COMMAND(InitSession(Session));

			ADD_LATENT_AUTOMATION_COMMAND(StartSessionByScene(Session, Auth, Scene));
			ADD_LATENT_AUTOMATION_COMMAND(WaitUntilConnectingComplete(Session));
			ADD_LATENT_AUTOMATION_COMMAND(TestEqualConnectionState(this, Session, EInworldConnectionState::Connected));

			ADD_LATENT_AUTOMATION_COMMAND(StopSession(Session));
			ADD_LATENT_AUTOMATION_COMMAND(WaitUntilDisconnectingComplete(Session));
			ADD_LATENT_AUTOMATION_COMMAND(TestEqualConnectionState(this, Session, EInworldConnectionState::Idle));

			ADD_LATENT_AUTOMATION_COMMAND(DestroySession(Session));

			return true;
		}
	}
}
