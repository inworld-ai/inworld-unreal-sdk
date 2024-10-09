/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "Tests/Interaction/Character/InworldTestSendPushToTalkAudioMessageToCharacter.h"
#include "Commands/InworldTestCommandsGarbageCollection.h"
#include "Commands/InworldTestCommandsCharacter.h"
#include "Commands/InworldTestCommandsSession.h"
#include "Commands/InworldTestCommandsInteraction.h"

bool Inworld::Test::FSendPushToTalkAudioMessageToCharacter::RunTest(const FString& Parameters)
{
	TScopedGCObject<UInworldTestObjectSession> TestObject;
	{
		FScopedSessionScene SessionScenePinned(TestObject->Session, TestObject->Scene, TestObject->Workspace, TestObject->RuntimeAuth);
		{
			FScopedSpeechProcessor SpeechProcessorPinned(TestObject->Session);
			{
				{
					FScopedCharacterAudioSession CharacterAudioSessionPin(TestObject->Characters[0], { EInworldMicrophoneMode::EXPECT_AUDIO_END });

					SendCharacterTestAudioData(TestObject->Characters[0]);

					SendCharacterBlankAudioData(TestObject->Characters[0]);

					TestInteractionEndFalse(TestObject->ControlEvents, 1);
				}

				WaitUntilInteractionEndWithTimeout(TestObject->ControlEvents, 1);
			}
		}

		TestTextEventCollection(TestObject->TextEvents);
		TestAudioDataEventCollection(TestObject->AudioDataEvents);
	}

	return true;
}
