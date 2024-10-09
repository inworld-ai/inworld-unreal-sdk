/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "Tests/SessionManagement/InworldTestLoadCharacters.h"
#include "Commands/InworldTestCommandsGarbageCollection.h"
#include "Commands/InworldTestCommandsCharacter.h"
#include "Commands/InworldTestCommandsSession.h"
#include "InworldAITestSettings.h"

UInworldTestObjectLoadCharacters::UInworldTestObjectLoadCharacters()
	: UInworldTestObjectSession()
{
	const UInworldAITestSettings* InworldAITestSettings = GetDefault<UInworldAITestSettings>();
	for (const FString& CharacterName : InworldAITestSettings->CharacterNamesToLoad)
	{
		UInworldCharacter* const Character = LoadCharacters.Emplace_GetRef(NewObject<UInworldCharacter>());
		Character->SetBrainName(CharacterName);
	}
}

bool Inworld::Test::FLoadCharacters::RunTest(const FString& Parameters)
{
	TScopedGCObject<UInworldTestObjectLoadCharacters> TestObject;
	{
		FScopedSessionScene SessionScenePinned(TestObject->Session, TestObject->Scene, TestObject->Workspace, TestObject->RuntimeAuth);

		for (UInworldCharacter* const Character : TestObject->LoadCharacters)
		{
			SetCharacterSession(Character, TestObject->Session);
		}

		for (UInworldCharacter* const Character : TestObject->LoadCharacters)
		{
			WaitUntilCharacterPossessedWithTimeout(Character);
		}
	}
	return true;
}