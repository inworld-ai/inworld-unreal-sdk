/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "Tests/Interaction/MultiConversation/InworldTestSendTextMessageToMultiConversation.h"
#include "Commands/InworldTestCommandsGarbageCollection.h"
#include "Commands/InworldTestCommandsPlayer.h"
#include "Commands/InworldTestCommandsSession.h"
#include "Commands/InworldTestCommandsInteraction.h"

bool Inworld::Test::FSendTextMessageToMultiConversation::RunTest(const FString& Parameters)
{
	TScopedGCObject<UInworldTestObjectSession> TestObject;
	{
		FScopedSessionScene SessionScenePinned(TestObject->Session, TestObject->SceneName, TestObject->RuntimeAuth);

		for (UInworldCharacter* const Character : TestObject->Characters)
		{
			AddPlayerTargetCharacter(TestObject->Player, Character);
		}

		const int32 NumMessages = 5;

		for (int32 i = 0; i < NumMessages; ++i)
		{
			SendTextMessageToConversation(TestObject->Player, TEXT("Hello!"));

			WaitUntilInteractionEndWithTimeout(TestObject->ControlEvents, i + 1);
		}

		TestTextEventCollection(TestObject->TextEvents);
		TestAudioDataEventCollection(TestObject->AudioDataEvents);
	}

	return true;
}
