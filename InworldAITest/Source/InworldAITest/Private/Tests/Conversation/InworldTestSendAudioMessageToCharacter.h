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
#include "InworldTestSendAudioMessageToCharacter.generated.h"

UCLASS()
class UInworldTestObjectSendAudioMessageToCharacter : public UInworldTestObjectSession
{
	GENERATED_BODY()
};

namespace Inworld
{
	namespace Test
	{
		IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSendAudioMessageToCharacter, "Inworld.Conversation.SendAudioMessageToCharacter", Flags)
		bool FSendAudioMessageToCharacter::RunTest(const FString& Parameters)
		{
			TScopedGCObject<UInworldTestObjectSendAudioMessageToCharacter> TestObject;
			{
				FScopedSessionScene SessionScenePinned(this, TestObject->Session, TestObject->SceneName, TestObject->RuntimeAuth);
				{
					FScopedSpeechProcessor SpeechProcessorPinned(TestObject->Session, EInworldPlayerSpeechMode::DEFAULT);
					{
						FScopedCharacterAudioSession CharacterAudioSessionPin(TestObject->Characters[0]);
						SendCharacterTestAudioData(TestObject->Characters[0]);
					}
				}

				WaitUntilInteractionEnd(TestObject->ControlEvents);

				TestTextEventCollection(this, TestObject->TextEvents);
				TestAudioDataEventCollection(this, TestObject->AudioDataEvents);
			}

			return true;
		}
	}
}
