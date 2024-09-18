/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "Tests/Interaction/Conversation/InworldTestSendOpenMicAudioMessageToConversation.h"
#include "Commands/InworldTestCommandsGarbageCollection.h"
#include "Commands/InworldTestCommandsPlayer.h"
#include "Commands/InworldTestCommandsSession.h"
#include "Commands/InworldTestCommandsInteraction.h"

bool Inworld::Test::FSendOpenMicAudioMessageToConversation::RunTest(const FString& Parameters)
{
	TScopedGCObject<UInworldTestObjectSendOpenMicAudioMessageToConversation> TestObject;
	{
		FScopedSessionScene SessionScenePinned(TestObject->Session, TestObject->SceneName, TestObject->RuntimeAuth);

		AddPlayerTargetCharacter(TestObject->Player, TestObject->Characters[0]);

		{
			FScopedSpeechProcessor SpeechProcessorPinned(TestObject->Session);
			{
				FScopedConversationAudioSession ConversationAudioSessionPin(TestObject->Player, { EInworldMicrophoneMode::OPEN_MIC });

				SendTestAudioDataToConversation(TestObject->Player);

				SendBlankAudioDataToConversation(TestObject->Player, 2.5f);

				WaitUntilInteractionEndWithTimeout(TestObject->ControlEvents, 1, 10.0f);
			}
		}

		TestTextEventCollection(TestObject->TextEvents);
		TestAudioDataEventCollection(TestObject->AudioDataEvents);
	}

	return true;
}
