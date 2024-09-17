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
#include "InworldTestSendTextMessageToConversation.generated.h"

UCLASS()
class UInworldTestObjectSendTextMessageToConversation : public UInworldTestObjectSession
{
	GENERATED_BODY()
};

namespace Inworld
{
	namespace Test
	{
		IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSendTextMessageToConversation, "Inworld.Interaction.Conversation.SendTextMessage", Flags)
		bool FSendTextMessageToConversation::RunTest(const FString& Parameters)
		{
			TScopedGCObject<UInworldTestObjectSendTextMessageToConversation> TestObject;
			{
				FScopedSessionScene SessionScenePinned(TestObject->Session, TestObject->SceneName, TestObject->RuntimeAuth);

				AddPlayerTargetCharacter(TestObject->Player, TestObject->Characters[0]);

				SendTextMessageToConversation(TestObject->Player, TEXT("Hello!"));

				WaitUntilInteractionEndWithTimeout(TestObject->ControlEvents, 1, 5.0f);

				TestTextEventCollection(TestObject->TextEvents);
				TestAudioDataEventCollection(TestObject->AudioDataEvents);
			}

			return true;
		}
	}
}
