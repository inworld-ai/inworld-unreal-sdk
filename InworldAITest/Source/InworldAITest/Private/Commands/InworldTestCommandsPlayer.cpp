/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "Commands/InworldTestCommandsPlayer.h"
#include "Commands/InworldTestCommandsWait.h"

#include "InworldTestUtils.h"

bool Inworld::Test::FAddPlayerTargetCharacterCommand::Update()
{
	Player->AddTargetCharacter(Character);
	return true;
}

bool Inworld::Test::FSendTextMessageToConversationCommand::Update()
{
	Player->SendTextMessageToConversation(Text);
	return true;
}

bool Inworld::Test::FSendAudioSessionStartToConversationCommand::Update()
{
	Player->SendAudioSessionStartToConversation(AudioSessionOptions);
	return true;
}

bool Inworld::Test::FSendAudioDataToConversationCommand::Update()
{
	Player->SendSoundMessageToConversation(AudioChunk, {});
	return true;
}

bool Inworld::Test::FSendAudioSessionStopToConversationCommand::Update()
{
	Player->SendAudioSessionStopToConversation();
	return true;
}

Inworld::Test::FScopedConversationAudioSession::FScopedConversationAudioSession(UInworldPlayer* InPlayer, const FInworldAudioSessionOptions& InAudioSessionOptions)
	: Player(InPlayer)
{
	SendAudioSessionStartToConversation(Player, InAudioSessionOptions);
}

Inworld::Test::FScopedConversationAudioSession::~FScopedConversationAudioSession()
{
	SendAudioSessionStopToConversation(Player);
}

void Inworld::Test::SendTestAudioDataToConversation(UInworldPlayer* Player)
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

void Inworld::Test::SendBlankAudioDataToConversation(UInworldPlayer* Player, float Duration)
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
