/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "InworldTestMacros.h"
#include "InworldTestChecks.h"

#include "InworldSession.h"

namespace Inworld
{
	namespace Test
	{
		DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(InitSpeechProcessor, UInworldSession*, Session, EInworldPlayerSpeechMode, Mode, FInworldPlayerSpeechOptions, SpeechOptions);
		bool FInitSpeechProcessorCommand::Update()
		{
			Session->InitSpeechProcessor(Mode, SpeechOptions);
			return true;
		}

		DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(DestroySpeechProcessor, UInworldSession*, Session);
		bool FDestroySpeechProcessorCommand::Update()
		{
			Session->DestroySpeechProcessor();
			return true;
		}

		DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(StartSessionByScene, UInworldSession*, Session, const FInworldAuth&, Auth, const FString&, SceneId);
		bool FStartSessionBySceneCommand::Update()
		{
			Session->StartSession({}, Auth, SceneId, {}, {}, {}, {});
			return true;
		}

		DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(StopSession, UInworldSession*, Session);
		bool FStopSessionCommand::Update()
		{
			Session->StopSession();
			return true;
		}

		DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(WaitUntilSessionConnectingComplete, UInworldSession*, Session);
		bool FWaitUntilSessionConnectingCompleteCommand::Update()
		{
			const EInworldConnectionState ConnectionState = Session->GetConnectionState();
			return ConnectionState != EInworldConnectionState::Idle && ConnectionState != EInworldConnectionState::Connecting;
		}

		DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(WaitUntilSessionDisconnectingComplete, UInworldSession*, Session);
		bool FWaitUntilSessionDisconnectingCompleteCommand::Update()
		{
			const EInworldConnectionState ConnectionState = Session->GetConnectionState();
			return ConnectionState != EInworldConnectionState::Connected;
		}

		DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(TestEqualConnectionState, UInworldSession*, Session, EInworldConnectionState, ConnectionState);
		bool FTestEqualConnectionStateCommand::Update()
		{
			static const UEnum* TypeEnum = StaticEnum<EInworldConnectionState>();
			const FName ExpectedTypeName = TypeEnum->GetNameByValue((int64)ConnectionState);
			const FName CurrentTypeName = TypeEnum->GetNameByValue((int64)Session->GetConnectionState());
			CheckEqual(TEXT("Connection State"), *CurrentTypeName.ToString(), *ExpectedTypeName.ToString());
			return true;
		}

		DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(WaitUntilSessionLoaded, UInworldSession*, Session);
		bool FWaitUntilSessionLoadedCommand::Update()
		{
			return Session->IsLoaded();
		}

		struct FScopedSessionScene
		{
			FScopedSessionScene(UInworldSession* InSession, const FString& InSceneName, const FInworldAuth& InRuntimeAuth)
				: Session(InSession)
			{
				StartSessionByScene(Session, InRuntimeAuth, InSceneName);
				WaitUntilSessionConnectingCompleteWithTimeout(Session, 10.0f);
				WaitUntilSessionLoadedWithTimeout(Session, 10.0f);
				TestEqualConnectionState(Session, EInworldConnectionState::Connected);
			}
			~FScopedSessionScene()
			{
				StopSession(Session);
				WaitUntilSessionDisconnectingCompleteWithTimeout(Session, 10.0f);
				TestEqualConnectionState(Session, EInworldConnectionState::Idle);
			}
		private:
			UInworldSession* Session;
		};

		struct FScopedSpeechProcessor
		{
			FScopedSpeechProcessor(UInworldSession* InSession, EInworldPlayerSpeechMode InPlayerSpeechMode = EInworldPlayerSpeechMode::DEFAULT, FInworldPlayerSpeechOptions InPlayerSpeechOptions = {})
				: Session(InSession)
			{
				InitSpeechProcessor(Session, InPlayerSpeechMode, InPlayerSpeechOptions);
			}
			~FScopedSpeechProcessor()
			{
				DestroySpeechProcessor(Session);
			}
		private:
			UInworldSession* Session;
		};
	}
}
