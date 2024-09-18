/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "InworldTestMacros.h"

#include "InworldCharacter.h"
#include "InworldSession.h"

namespace Inworld
{
	namespace Test
	{
		DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(SetCharacterSession, UInworldCharacter*, Character, UInworldSession*, Session);

		DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(WaitUntilCharacterPossessed, UInworldCharacter*, Character);

		DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(WaitUntilCharacterUnpossessed, UInworldCharacter*, Character);

		DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(SendCharacterTextMessage, UInworldCharacter*, Character, FString, Text);

		DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(SendCharacterAudioSessionStart, UInworldCharacter*, Character, FInworldAudioSessionOptions, AudioSessionOptions);

		DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(SendCharacterAudioData, UInworldCharacter*, Character, TArray<uint8>, AudioChunk);

		DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(SendCharacterAudioSessionStop, UInworldCharacter*, Character);

		struct FScopedCharacterAudioSession
		{
			FScopedCharacterAudioSession(UInworldCharacter* InCharacter, const FInworldAudioSessionOptions& InAudioSessionOptions = {});
			~FScopedCharacterAudioSession();
		private:
			UInworldCharacter* Character;
		};

		void SendCharacterTestAudioData(UInworldCharacter* Character);
		void SendCharacterBlankAudioData(UInworldCharacter* Character, float Duration);
	}
}
