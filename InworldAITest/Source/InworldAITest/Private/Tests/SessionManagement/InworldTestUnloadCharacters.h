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
#include "InworldTestUnloadCharacters.generated.h"

UCLASS()
class UInworldTestObjectUnloadCharacters : public UInworldTestObjectSession
{
	GENERATED_BODY()
public:
	UInworldTestObjectUnloadCharacters()
		: UInworldTestObjectSession()
		, UnloadCharacters(Characters.FilterByPredicate(
			[](UInworldCharacter* Character) -> bool
			{
				return GetDefault<UInworldAITestSettings>()->CharacterNamesToUnload.Contains(Character->GetAgentInfo().BrainName);
			}
		))
	{
	}

	UPROPERTY()
	TArray<TObjectPtr<UInworldCharacter>> UnloadCharacters;
};

namespace Inworld
{
	namespace Test
	{
		IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUnloadCharacters, "Inworld.SessionManagement.UnloadCharacters", Flags)
		bool FUnloadCharacters::RunTest(const FString& Parameters)
		{
			TScopedGCObject<UInworldTestObjectUnloadCharacters> TestObject;
			{
				FScopedSessionScene SessionScenePinned(TestObject->Session, TestObject->SceneName, TestObject->RuntimeAuth);

				for (UInworldCharacter* const Character : TestObject->UnloadCharacters)
				{
					SetCharacterSession(Character, nullptr);
				}

				for (UInworldCharacter* const Character : TestObject->UnloadCharacters)
				{
					WaitUntilCharacterUnpossessedWithTimeout(Character, 10.0f);
				}
			}
			return true;
		}
	}
}
