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
#include "InworldTestSendOpenMicAudioMessageToConversation.generated.h"

UCLASS()
class UInworldTestObjectSendOpenMicAudioMessageToConversation : public UInworldTestObjectSession
{
	GENERATED_BODY()
};

namespace Inworld
{
	namespace Test
	{
		IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSendOpenMicAudioMessageToConversation, "Inworld.Interaction.Conversation.SendOpenMicAudioMessage", Flags)
		bool FSendOpenMicAudioMessageToConversation::RunTest(const FString& Parameters)
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
	}
}
