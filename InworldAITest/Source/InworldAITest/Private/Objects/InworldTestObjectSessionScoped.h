/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "InworldTestCommands.h"
#include "Objects/InworldTestObjectSession.h"
#include "Objects/InworldTestObjectScoped.h"


namespace Inworld
{
	namespace Test
	{
		template<typename T>
		struct TInworldTestObjectSessionScoped : public TInworldTestObjectScoped<T>
		{
		public:
			TInworldTestObjectSessionScoped(FAutomationTestBase* Test)
				: TInworldTestObjectScoped(Test)
			{
				T& Object = Get();
				ADD_LATENT_AUTOMATION_COMMAND(StartSessionByScene(Object.Session, Object.RuntimeAuth, Object.Scene));
				ADD_LATENT_AUTOMATION_COMMAND(WaitUntilSessionConnectingComplete(Object.Session));
				ADD_LATENT_AUTOMATION_COMMAND(WaitUntilSessionLoaded(Object.Session));
				ADD_LATENT_AUTOMATION_COMMAND(TestEqualConnectionState(OwningTest, Object.Session, EInworldConnectionState::Connected));
			}

			~TInworldTestObjectSessionScoped()
			{
				T& Object = Get();
				ADD_LATENT_AUTOMATION_COMMAND(StopSession(Object.Session));
				ADD_LATENT_AUTOMATION_COMMAND(WaitUntilSessionDisconnectingComplete(Object.Session));
				ADD_LATENT_AUTOMATION_COMMAND(TestEqualConnectionState(OwningTest, Object.Session, EInworldConnectionState::Idle));
			}
		};
	}
}
