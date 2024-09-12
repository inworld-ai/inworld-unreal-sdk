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

		DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(SendCharacterAudioSessionStart, UInworldCharacter*, Character, FInworldAudioSessionOptions, AudioSessionOptions);
		bool SendCharacterAudioSessionStart::Update()
		{
			Character->SendAudioSessionStart(AudioSessionOptions);
			return true;
		}

		DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(SendCharacterAudioData, UInworldCharacter*, Character, TArray<uint8>, AudioChunk);
		bool SendCharacterAudioData::Update()
		{
			Character->SendSoundMessage(AudioChunk, {});
			return true;
		}

		DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(SendCharacterAudioSessionStop, UInworldCharacter*, Character);
		bool SendCharacterAudioSessionStop::Update()
		{
			Character->SendAudioSessionStop();
			return true;
		}
	}
}
