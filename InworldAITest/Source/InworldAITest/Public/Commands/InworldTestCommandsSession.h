/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "InworldTestMacros.h"

#include "InworldSession.h"

namespace Inworld
{
	namespace Test
	{
		DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(InitSpeechProcessor, UInworldSession*, Session, EInworldPlayerSpeechMode, Mode, FInworldPlayerSpeechOptions, SpeechOptions);

		DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(DestroySpeechProcessor, UInworldSession*, Session);

		DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_FOUR_PARAMETER(StartSessionByScene, UInworldSession*, Session, const FInworldScene&, Scene, const FString&, Workspace, const FInworldAuth&, Auth);

		DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(StopSession, UInworldSession*, Session);

		DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(WaitUntilSessionConnectingComplete, UInworldSession*, Session);

		DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(WaitUntilSessionDisconnectingComplete, UInworldSession*, Session);

		DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(TestEqualConnectionState, UInworldSession*, Session, EInworldConnectionState, ConnectionState);

		DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(WaitUntilSessionLoaded, UInworldSession*, Session);

		struct FScopedSessionScene
		{
			FScopedSessionScene(UInworldSession* InSession, const FInworldScene& InScene, const FString& InWorkspace, const FInworldAuth& InRuntimeAuth);
			~FScopedSessionScene();
		private:
			UInworldSession* Session;
		};

		struct FScopedSpeechProcessor
		{
			FScopedSpeechProcessor(UInworldSession* InSession, EInworldPlayerSpeechMode InPlayerSpeechMode = EInworldPlayerSpeechMode::DEFAULT, FInworldPlayerSpeechOptions InPlayerSpeechOptions = {});
			~FScopedSpeechProcessor();
		private:
			UInworldSession* Session;
		};
	}
}
