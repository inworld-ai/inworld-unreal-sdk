/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Objects/InworldTestObjectSession.h"
#include "Objects/InworldTestObjectSessionScoped.h"
#include "InworldTestFlags.h"
#include "InworldTestCommands.h"
#include "InworldTestSendTextMessageToCharacter.generated.h"

UCLASS()
class UInworldTestObjectSendTextMessageToCharacter : public UInworldTestObjectSession
{
	GENERATED_BODY()

public:
	UInworldTestObjectSendTextMessageToCharacter()
		: UInworldTestObjectSession({ TEXT("workspaces/sdk_test_automation/characters/character_one"), { TEXT("character_one"), } })
	{
	}
};

namespace Inworld
{
	namespace Test
	{
		IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSendTextMessageToCharacter, "Inworld.Conversation.SendTextMessageToCharacter", Flags)
		bool FSendTextMessageToCharacter::RunTest(const FString& Parameters)
		{
			TInworldTestObjectSessionScoped<UInworldTestObjectSendTextMessageToCharacter> TestObject(this);

			ADD_LATENT_AUTOMATION_COMMAND(SendCharacterTextMessage(TestObject->Characters[0], TEXT("Hello!")));
			ADD_LATENT_AUTOMATION_COMMAND(WaitUntilInteractionEnd(TestObject->ControlEvents));
			ADD_LATENT_AUTOMATION_COMMAND(TestTextEventCollectionNotEmpty(this, TestObject->TextEvents))
			ADD_LATENT_AUTOMATION_COMMAND(TestAudioDataEventCollectionNotEmpty(this, TestObject->AudioDataEvents))
			ADD_LATENT_AUTOMATION_COMMAND(TestTextEventCollectionValid(this, TestObject->TextEvents))
			ADD_LATENT_AUTOMATION_COMMAND(TestAudioDataEventCollectionValid(this, TestObject->AudioDataEvents))

			return true;
		}
	}
}