/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "Tests/Interaction/Conversation/InworldTestSendTextMessageToConversation.h"
#include "Commands/InworldTestCommandsGarbageCollection.h"
#include "Commands/InworldTestCommandsPlayer.h"
#include "Commands/InworldTestCommandsSession.h"
#include "Commands/InworldTestCommandsInteraction.h"

bool Inworld::Test::FSendTextMessageToConversation::RunTest(const FString& Parameters)
{
	TScopedGCObject<UInworldTestObjectSession> TestObject;
	{
		FScopedSessionScene SessionScenePinned(TestObject->Session, TestObject->Scene, TestObject->Workspace, TestObject->RuntimeAuth);

		AddPlayerTargetCharacter(TestObject->Player, TestObject->Characters[0]);

		SendTextMessageToConversation(TestObject->Player, TEXT("Hello!"));

		WaitUntilInteractionEndWithTimeout(TestObject->ControlEvents, 1);

		TestTextEventCollection(TestObject->TextEvents);
		TestAudioDataEventCollection(TestObject->AudioDataEvents);
	}

	return true;
}
