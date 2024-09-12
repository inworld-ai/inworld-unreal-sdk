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

#include "InworldCharacter.h"

#include "InworldAITestSettings.h"

namespace Inworld
{
	namespace Test
	{
		DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(SendCharacterTextMessage, UInworldCharacter*, Character, FString, Text);
		bool SendCharacterTextMessage::Update()
		{
			Character->SendTextMessage(Text);
			return true;
		}
	}
}
