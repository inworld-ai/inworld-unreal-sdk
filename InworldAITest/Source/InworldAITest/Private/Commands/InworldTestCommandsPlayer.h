/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "InworldTestMacros.h"
#include "InworldTestUtils.h"

#include "InworldPlayer.h"

#include "Commands/InworldTestCommandsWait.h"

#include "InworldAITestSettings.h"

namespace Inworld
{
	namespace Test
	{
		DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(AddPlayerTargetCharacter, UInworldPlayer*, Player, UInworldCharacter*, Character);
		bool FAddPlayerTargetCharacterCommand::Update()
		{
			Player->AddTargetCharacter(Character);
			return true;
		}

		DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(SendTextMessageToConversation, UInworldPlayer*, Player, FString, Text);
		bool FSendTextMessageToConversationCommand::Update()
		{
			Player->SendTextMessageToConversation(Text);
			return true;
		}

		DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(SendAudioSessionStartToConversation, UInworldPlayer*, Player, FInworldAudioSessionOptions, AudioSessionOptions);
		bool FSendAudioSessionStartToConversationCommand::Update()
		{
			Player->SendAudioSessionStartToConversation(AudioSessionOptions);
			return true;
		}

		DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(SendAudioDataToConversation, UInworldPlayer*, Player, TArray<uint8>, AudioChunk);
		bool FSendAudioDataToConversationCommand::Update()
		{
			Player->SendSoundMessageToConversation(AudioChunk, {});
			return true;
		}

		DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(SendAudioSessionStopToConversation, UInworldPlayer*, Player);
		bool FSendAudioSessionStopToConversationCommand::Update()
		{
			Player->SendAudioSessionStopToConversation();
			return true;
		}

		struct FScopedConversationAudioSession
		{
			FScopedConversationAudioSession(UInworldPlayer* InPlayer, const FInworldAudioSessionOptions& InAudioSessionOptions = {})
				: Player(InPlayer)
			{
				SendAudioSessionStartToConversation(Player, InAudioSessionOptions);
			}
			~FScopedConversationAudioSession()
			{
				SendAudioSessionStopToConversation(Player);
			}
		private:
			UInworldPlayer* Player;
		};

		void SendTestAudioDataToConversation(UInworldPlayer* Player)
		{
			TArray<uint8> TestAudioData = GetTestAudioData();
			constexpr int32 MaxChunkSize = (16000 / 10) * 2;
			for (int32 i = 44; i < TestAudioData.Num(); i += MaxChunkSize)
			{
				const int32 AudioChunkSize = FMath::Min(MaxChunkSize, TestAudioData.Num() - i);
				const TArray<uint8> AudioChunk(TestAudioData.GetData() + i, AudioChunkSize);
				SendAudioDataToConversation(Player, AudioChunk);
				Wait(0.1f);
			}
		}

		void SendBlankAudioDataToConversation(UInworldPlayer* Player, float Duration)
		{
			constexpr int32 MaxChunkSize = (16000 / 10) * 2;
			TArray<uint8> AudioChunk;
			AudioChunk.SetNumZeroed(MaxChunkSize);
			while (Duration > 0.f)
			{
				SendAudioDataToConversation(Player, AudioChunk);
				Wait(0.1f);
				Duration -= 0.1f;
			}
		}
	}
}
