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

#include "InworldSession.h"

#include "InworldAITestSettings.h"

namespace Inworld
{
	namespace Test
	{
		DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(StartSessionByScene, UInworldSession*, Session, const FInworldAuth&, Auth, const FString&, SceneId);
		bool StartSessionByScene::Update()
		{
			Session->StartSession({}, Auth, SceneId, {}, {}, {}, {});
			return true;
		}

		DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(StopSession, UInworldSession*, Session);
		bool StopSession::Update()
		{
			Session->StopSession();
			return true;
		}

		DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(WaitUntilSessionConnectingComplete, UInworldSession*, Session);
		bool WaitUntilSessionConnectingComplete::Update()
		{
			const EInworldConnectionState ConnectionState = Session->GetConnectionState();
			return ConnectionState != EInworldConnectionState::Idle && ConnectionState != EInworldConnectionState::Connecting;
		}

		DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(WaitUntilSessionDisconnectingComplete, UInworldSession*, Session);
		bool WaitUntilSessionDisconnectingComplete::Update()
		{
			const EInworldConnectionState ConnectionState = Session->GetConnectionState();
			return ConnectionState != EInworldConnectionState::Connected;
		}

		DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(TestEqualConnectionState, FAutomationTestBase*, Test, UInworldSession*, Session, EInworldConnectionState, ConnectionState);
		bool TestEqualConnectionState::Update()
		{
			static const UEnum* TypeEnum = StaticEnum<EInworldConnectionState>();
			const FName ExpectedTypeName = TypeEnum->GetNameByValue((int64)ConnectionState);
			const FName CurrentTypeName = TypeEnum->GetNameByValue((int64)Session->GetConnectionState());
			Test->TestEqual(TEXT("Connection State"), *CurrentTypeName.ToString(), *ExpectedTypeName.ToString());
			return true;
		}

		DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(WaitUntilSessionLoaded, UInworldSession*, Session);
		bool WaitUntilSessionLoaded::Update()
		{
			return Session->IsLoaded();
		}
	}
}
