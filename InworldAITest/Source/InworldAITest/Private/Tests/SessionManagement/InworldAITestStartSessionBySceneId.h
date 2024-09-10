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
#include "InworldAITestStartSessionBySceneId.generated.h"

USTRUCT()
struct FInworldAITestStartSessionBySceneId
{
	GENERATED_BODY()
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInworldStartSessionBySceneId, "Inworld.SessionManagement.StartSessionBySceneId", Inworld::Test::Flags)
bool FInworldStartSessionBySceneId::RunTest(const FString& Parameters)
{
	const FString SceneId{ TEXT("workspaces/sdk_test_automation/scenes/full_scene") };

	FInworldAuth Auth = Inworld::Test::GetAuth();

	UInworldSession* Session = NewObject<UInworldSession>();
	ADD_LATENT_AUTOMATION_COMMAND(FInworldInitSession(Session));

	ADD_LATENT_AUTOMATION_COMMAND(FInworldStartSessionByScene(Session, Auth, SceneId));
	ADD_LATENT_AUTOMATION_COMMAND(FInworldWaitUntilConnectingComplete(Session));
	ADD_LATENT_AUTOMATION_COMMAND(FInworldAITestEqualConnectionState(this, Session, EInworldConnectionState::Connected));

	ADD_LATENT_AUTOMATION_COMMAND(FInworldStopSession(Session));
	ADD_LATENT_AUTOMATION_COMMAND(FInworldWaitUntilDisconnectingComplete(Session));
	ADD_LATENT_AUTOMATION_COMMAND(FInworldAITestEqualConnectionState(this, Session, EInworldConnectionState::Idle));

	ADD_LATENT_AUTOMATION_COMMAND(FInworldDestroySession(Session));

	return true;
}
