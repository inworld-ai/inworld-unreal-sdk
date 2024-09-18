/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "Tests/SessionManagement/InworldTestStartSessionByScene.h"
#include "Commands/InworldTestCommandsGarbageCollection.h"
#include "Commands/InworldTestCommandsSession.h"

bool Inworld::Test::FStartSessionByScene::RunTest(const FString& Parameters)
{
	TScopedGCObject<UInworldTestObjectSession> TestObject;
	{
		FScopedSessionScene SessionScenePinned(TestObject->Session, TestObject->SceneName, TestObject->RuntimeAuth);
	}
	return true;
}
