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

namespace Inworld
{
	namespace Test
	{
		DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(InitSession, UInworldSession*, Session);
		bool InitSession::Update()
		{
			Session->Init();
			return true;
		}

		DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(DestroySession, UInworldSession*, Session);
		bool DestroySession::Update()
		{
			Session->MarkAsGarbage();
			return true;
		}

		DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(StartSessionByScene, UInworldSession*, Session, FInworldAuth, Auth, FString, SceneId);
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

		DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(WaitUntilConnectingComplete, UInworldSession*, Session);
		bool WaitUntilConnectingComplete::Update()
		{
			const EInworldConnectionState ConnectionState = Session->GetConnectionState();
			return ConnectionState != EInworldConnectionState::Idle && ConnectionState != EInworldConnectionState::Connecting;
		}

		DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(WaitUntilDisconnectingComplete, UInworldSession*, Session);
		bool WaitUntilDisconnectingComplete::Update()
		{
			const EInworldConnectionState ConnectionState = Session->GetConnectionState();
			return ConnectionState != EInworldConnectionState::Connected;
		}

		DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(TestEqualConnectionState, FAutomationTestBase*, Test, UInworldSession*, Session, EInworldConnectionState, ConnectionState);
		bool TestEqualConnectionState::Update()
		{
			static const UEnum* TypeEnum = StaticEnum<EInworldConnectionState>();
			const FName TypeName = TypeEnum->GetNameByValue((int64)ConnectionState);
			Test->TestEqual(FString::Printf(TEXT("Check that connection state is %s"), *TypeName.ToString()), Session->GetConnectionState(), ConnectionState);
			return true;
		}

	}
}
