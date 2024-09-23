/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "Commands/InworldTestCommandsCharacter.h"
#include "Commands/InworldTestCommandsWait.h"
#include "InworldTestUtils.h"

bool Inworld::Test::FSetCharacterSessionCommand::Update()
{
	Character->SetSession(Session);
	return true;
}

bool Inworld::Test::FWaitUntilCharacterPossessedCommand::Update()
{
	return Character->IsPossessed();
}

bool Inworld::Test::FWaitUntilCharacterUnpossessedCommand::Update()
{
	return !Character->IsPossessed();
}

bool Inworld::Test::FSendCharacterTextMessageCommand::Update()
{
	Character->SendTextMessage(Text);
	return true;
}

bool Inworld::Test::FSendCharacterAudioSessionStartCommand::Update()
{
	Character->SendAudioSessionStart(AudioSessionOptions);
	return true;
}

bool Inworld::Test::FSendCharacterAudioDataCommand::Update()
{
	Character->SendSoundMessage(AudioChunk, {});
	return true;
}

bool Inworld::Test::FSendCharacterAudioSessionStopCommand::Update()
{
	Character->SendAudioSessionStop();
	return true;
}

Inworld::Test::FScopedCharacterAudioSession::FScopedCharacterAudioSession(UInworldCharacter* InCharacter, const FInworldAudioSessionOptions& InAudioSessionOptions)
	: Character(InCharacter)
{
	SendCharacterAudioSessionStart(Character, InAudioSessionOptions);
}

Inworld::Test::FScopedCharacterAudioSession::~FScopedCharacterAudioSession()
{
	SendCharacterAudioSessionStop(Character);
}

void Inworld::Test::SendCharacterTestAudioData(UInworldCharacter* Character)
{
	TArray<uint8> TestAudioData = GetTestAudioData();
	constexpr int32 MaxChunkSize = (16000 / 10) * 2;
	for (int32 i = 44; i < TestAudioData.Num(); i += MaxChunkSize)
	{
		const int32 AudioChunkSize = FMath::Min(MaxChunkSize, TestAudioData.Num() - i);
		const TArray<uint8> AudioChunk(TestAudioData.GetData() + i, AudioChunkSize);
		SendCharacterAudioData(Character, AudioChunk);
		Wait(0.1f);
	}
}

void Inworld::Test::SendCharacterBlankAudioData(UInworldCharacter* Character, float Duration)
{
	constexpr int32 MaxChunkSize = (16000 / 10) * 2;
	TArray<uint8> AudioChunk;
	AudioChunk.SetNumZeroed(MaxChunkSize);
	while (Duration > 0.f)
	{
		SendCharacterAudioData(Character, AudioChunk);
		Wait(0.1f);
		Duration -= 0.1f;
	}
}