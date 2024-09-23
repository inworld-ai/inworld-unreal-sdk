/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "Tests/Interaction/MultiConversation/InworldTestSendPushToTalkAudioMessageToMultiConversation.h"
#include "Commands/InworldTestCommandsGarbageCollection.h"
#include "Commands/InworldTestCommandsPlayer.h"
#include "Commands/InworldTestCommandsSession.h"
#include "Commands/InworldTestCommandsInteraction.h"

bool Inworld::Test::FSendPushToTalkAudioMessageToMultiConversation::RunTest(const FString& Parameters)
{
	TScopedGCObject<UInworldTestObjectSession> TestObject;
	{
		FScopedSessionScene SessionScenePinned(TestObject->Session, TestObject->Scene, TestObject->Workspace, TestObject->RuntimeAuth);

		for (UInworldCharacter* const Character : TestObject->Characters)
		{
			AddPlayerTargetCharacter(TestObject->Player, Character);
		}

		const int32 NumMessages = 5;
		{
			FScopedSpeechProcessor SpeechProcessorPinned(TestObject->Session);

			for (int32 i = 0; i < NumMessages; ++i)
			{
				{
					FScopedConversationAudioSession ConversationAudioSessionPin(TestObject->Player, { EInworldMicrophoneMode::EXPECT_AUDIO_END });

					SendTestAudioDataToConversation(TestObject->Player);

					SendBlankAudioDataToConversation(TestObject->Player, 2.5f);

					TestInteractionEndFalse(TestObject->ControlEvents, i + 1);
				}

				WaitUntilInteractionEndWithTimeout(TestObject->ControlEvents, i + 1);
			}
		}

		TestTextEventCollection(TestObject->TextEvents);
		TestAudioDataEventCollection(TestObject->AudioDataEvents);
	}

	return true;
}
