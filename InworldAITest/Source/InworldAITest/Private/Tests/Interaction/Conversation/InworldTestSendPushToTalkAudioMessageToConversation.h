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
#include "InworldTestFlags.h"
#include "TestObjects/InworldTestObjectSession.h"
#include "Commands/InworldTestCommandsGarbageCollection.h"
#include "Commands/InworldTestCommandsPlayer.h"
#include "Commands/InworldTestCommandsInteraction.h"
#include "Commands/InworldTestCommandsWait.h"
#include "InworldTestSendPushToTalkAudioMessageToConversation.generated.h"

UCLASS()
class UInworldTestObjectSendPushToTalkAudioMessageToConversation : public UInworldTestObjectSession
{
	GENERATED_BODY()
};

namespace Inworld
{
	namespace Test
	{
		IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSendPushToTalkAudioMessageToConversation, "Inworld.Interaction.Conversation.SendPushToTalkAudioMessage", Flags)
		bool FSendPushToTalkAudioMessageToConversation::RunTest(const FString& Parameters)
		{
			TScopedGCObject<UInworldTestObjectSendPushToTalkAudioMessageToConversation> TestObject;
			{
				FScopedSessionScene SessionScenePinned(TestObject->Session, TestObject->SceneName, TestObject->RuntimeAuth);

				AddPlayerTargetCharacter(TestObject->Player, TestObject->Characters[0]);

				{
					FScopedSpeechProcessor SpeechProcessorPinned(TestObject->Session);
					{
						{
							FScopedConversationAudioSession ConversationAudioSessionPin(TestObject->Player, { EInworldMicrophoneMode::EXPECT_AUDIO_END });

							SendTestAudioDataToConversation(TestObject->Player);

							SendBlankAudioDataToConversation(TestObject->Player, 2.5f);

							TestInteractionEndFalse(TestObject->ControlEvents, 1);
						}

						WaitUntilInteractionEndWithTimeout(TestObject->ControlEvents, 1, 10.0f);
					}
				}

				TestTextEventCollection(TestObject->TextEvents);
				TestAudioDataEventCollection(TestObject->AudioDataEvents);
			}

			return true;
		}
	}
}
