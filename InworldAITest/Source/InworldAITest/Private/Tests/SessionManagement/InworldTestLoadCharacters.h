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
#include "InworldTestLoadCharacters.generated.h"

UCLASS()
class UInworldTestObjectLoadCharacters : public UInworldTestObjectSession
{
	GENERATED_BODY()
public:
	UInworldTestObjectLoadCharacters()
		: UInworldTestObjectSession()
	{
		const UInworldAITestSettings* InworldAITestSettings = GetDefault<UInworldAITestSettings>();
		for (const FString& CharacterName : InworldAITestSettings->CharacterNamesToLoad)
		{
			UInworldCharacter* const Character = LoadCharacters.Emplace_GetRef(NewObject<UInworldCharacter>());
			Character->SetBrainName(CharacterName);
		}
	}

	TArray<TObjectPtr<UInworldCharacter>> LoadCharacters;
};

namespace Inworld
{
	namespace Test
	{
		IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLoadCharacters, "Inworld.SessionManagement.LoadCharacters", Flags)
		bool FLoadCharacters::RunTest(const FString& Parameters)
		{
			TScopedGCObject<UInworldTestObjectLoadCharacters> TestObject;
			{
				FScopedSessionScene SessionScenePinned(TestObject->Session, TestObject->SceneName, TestObject->RuntimeAuth);

				for (UInworldCharacter* const Character : TestObject->LoadCharacters)
				{
					SetCharacterSession(Character, TestObject->Session);
				}

				for (UInworldCharacter* const Character : TestObject->LoadCharacters)
				{
					WaitUntilCharacterPossessedWithTimeout(Character, 10.0f);
				}
			}
			return true;
		}
	}
}
