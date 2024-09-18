/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "InworldTestMacros.h"

#include "InworldTypes.h"
#include "InworldPlayer.h"
#include "InworldCharacter.h"

namespace Inworld
{
	namespace Test
	{
		DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(AddPlayerTargetCharacter, UInworldPlayer*, Player, UInworldCharacter*, Character);

		DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(SendTextMessageToConversation, UInworldPlayer*, Player, FString, Text);

		DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(SendAudioSessionStartToConversation, UInworldPlayer*, Player, FInworldAudioSessionOptions, AudioSessionOptions);

		DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(SendAudioDataToConversation, UInworldPlayer*, Player, TArray<uint8>, AudioChunk);

		DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(SendAudioSessionStopToConversation, UInworldPlayer*, Player);

		struct FScopedConversationAudioSession
		{
			FScopedConversationAudioSession(UInworldPlayer* InPlayer, const FInworldAudioSessionOptions& InAudioSessionOptions = {});
			~FScopedConversationAudioSession();
		private:
			UInworldPlayer* Player;
		};

		void SendTestAudioDataToConversation(UInworldPlayer* Player);
		void SendBlankAudioDataToConversation(UInworldPlayer* Player, float Duration = INWORLD_TEST_BLANK_AUDIO_DURATION);
	}
}
