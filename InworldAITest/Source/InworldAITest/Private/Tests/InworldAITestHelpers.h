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

#include "InworldSession.h"
#include "InworldAITestSettings.h"

#include "InworldAITestHelpers.generated.h"


USTRUCT()
struct FInworldAITestHelpers
{
	GENERATED_BODY()
};

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FInworldInitSession, UInworldSession*, Session);
bool FInworldInitSession::Update()
{
	Session->Init();
	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FInworldDestroySession, UInworldSession*, Session);
bool FInworldDestroySession::Update()
{
	Session->MarkAsGarbage();
	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(FInworldStartSessionByScene, UInworldSession*, Session, FInworldAuth, Auth, FString, SceneId);
bool FInworldStartSessionByScene::Update()
{
	Session->StartSession({}, Auth, SceneId, {}, {}, {}, {}, {});
	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FInworldStopSession, UInworldSession*, Session);
bool FInworldStopSession::Update()
{
	Session->StopSession();
	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FInworldWaitUntilConnectingComplete, UInworldSession*, Session);
bool FInworldWaitUntilConnectingComplete::Update()
{
	const EInworldConnectionState ConnectionState = Session->GetConnectionState();
	return ConnectionState != EInworldConnectionState::Idle && ConnectionState != EInworldConnectionState::Connecting;
}

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FInworldWaitUntilDisconnectingComplete, UInworldSession*, Session);
bool FInworldWaitUntilDisconnectingComplete::Update()
{
	const EInworldConnectionState ConnectionState = Session->GetConnectionState();
	return ConnectionState != EInworldConnectionState::Connected;
}

DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(FInworldAITestEqualConnectionState, FAutomationTestBase*, Test, UInworldSession*, Session, EInworldConnectionState, ConnectionState);
bool FInworldAITestEqualConnectionState::Update()
{
	static const UEnum* TypeEnum = StaticEnum<EInworldConnectionState>();
	const FName TypeName = TypeEnum->GetNameByValue((int64)ConnectionState);
	Test->TestEqual(FString::Printf(TEXT("Check that connection state is %s"), *TypeName.ToString()), Session->GetConnectionState(), ConnectionState);
	return true;
}
