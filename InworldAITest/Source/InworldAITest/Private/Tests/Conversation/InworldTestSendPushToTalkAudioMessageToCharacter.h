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
#include "Commands/InworldTestCommandsCharacter.h"
#include "Commands/InworldTestCommandsInteraction.h"
#include "Commands/InworldTestCommandsWait.h"
#include "InworldTestSendPushToTalkAudioMessageToCharacter.generated.h"

UCLASS()
class UInworldTestObjectSendPushToTalkAudioMessageToCharacter : public UInworldTestObjectSession
{
	GENERATED_BODY()
};

namespace Inworld
{
	namespace Test
	{
		IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSendPushToTalkAudioMessageToCharacter, "Inworld.Conversation.SendPushToTalkAudioMessageToCharacter", Flags)
		bool FSendPushToTalkAudioMessageToCharacter::RunTest(const FString& Parameters)
		{
			TScopedGCObject<UInworldTestObjectSendPushToTalkAudioMessageToCharacter> TestObject;
			{
				FScopedSessionScene SessionScenePinned(TestObject->Session, TestObject->SceneName, TestObject->RuntimeAuth);
				{
					FScopedSpeechProcessor SpeechProcessorPinned(TestObject->Session);
					{
						FScopedCharacterAudioSession CharacterAudioSessionPin(TestObject->Characters[0], { EInworldMicrophoneMode::EXPECT_AUDIO_END });
						SendCharacterTestAudioData(TestObject->Characters[0]);

						Wait(5.0f);

						TestInteractionOngoing(TestObject->ControlEvents);
					}
				}

				WaitUntilInteractionEndWithTimeout(TestObject->ControlEvents, 10.0f);

				TestTextEventCollection(TestObject->TextEvents);
				TestAudioDataEventCollection(TestObject->AudioDataEvents);
			}

			return true;
		}
	}
}
