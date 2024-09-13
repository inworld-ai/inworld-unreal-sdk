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
#include "InworldTestMacros.h"
#include "InworldTestUtils.h"

#include "InworldCharacter.h"

#include "InworldAITestSettings.h"

namespace Inworld
{
	namespace Test
	{
		DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(SendCharacterTextMessage, UInworldCharacter*, Character, FString, Text);
		bool FSendCharacterTextMessageCommand::Update()
		{
			Character->SendTextMessage(Text);
			return true;
		}

		DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(SendCharacterAudioSessionStart, UInworldCharacter*, Character, FInworldAudioSessionOptions, AudioSessionOptions);
		bool FSendCharacterAudioSessionStartCommand::Update()
		{
			Character->SendAudioSessionStart(AudioSessionOptions);
			return true;
		}

		DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(SendCharacterAudioData, UInworldCharacter*, Character, TArray<uint8>, AudioChunk);
		bool FSendCharacterAudioDataCommand::Update()
		{
			Character->SendSoundMessage(AudioChunk, {});
			return true;
		}

		DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(SendCharacterAudioSessionStop, UInworldCharacter*, Character);
		bool FSendCharacterAudioSessionStopCommand::Update()
		{
			Character->SendAudioSessionStop();
			return true;
		}

		struct FScopedCharacterAudioSession
		{
			FScopedCharacterAudioSession(UInworldCharacter* InCharacter, const FInworldAudioSessionOptions& InAudioSessionOptions = FInworldAudioSessionOptions::Default())
				: Character(InCharacter)
			{
				SendCharacterAudioSessionStart(Character, InAudioSessionOptions);
			}
			~FScopedCharacterAudioSession()
			{
				SendCharacterAudioSessionStop(Character);
			}
		private:
			UInworldCharacter* Character;
		};

		void SendCharacterTestAudioData(UInworldCharacter* Character)
		{
			TArray<uint8> TestAudioData = GetTestAudioData();
			constexpr int32 MaxChunkSize = (16000 / 10) * 2;
			for (int32 i = 44; i < TestAudioData.Num(); i += MaxChunkSize)
			{
				const int32 AudioChunkSize = FMath::Min(MaxChunkSize, TestAudioData.Num() - i);
				const TArray<uint8> AudioChunk(TestAudioData.GetData() + i, AudioChunkSize);
				SendCharacterAudioData(Character, AudioChunk);
				ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(0.1f));
			}
		}
	}
}
