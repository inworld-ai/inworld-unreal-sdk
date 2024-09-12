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
#include "InworldEnums.h"
#include "InworldTypes.h"
#include "InworldTestFlags.h"
#include "InworldTestUtils.h"
#include "TestObjects/InworldTestObjectSession.h"
#include "Commands/InworldTestCommandsCharacter.h"
#include "Commands/InworldTestCommandsInteraction.h"
#include "InworldTestSendAudioMessageToCharacter.generated.h"

UCLASS()
class UInworldTestObjectSendAudioMessageToCharacter : public UInworldTestObjectSession
{
	GENERATED_BODY()

public:
	UInworldTestObjectSendAudioMessageToCharacter()
		: UInworldTestObjectSession()
		, TestAudioData(Inworld::Test::GetTestAudioData())
	{
	}

	TArray<uint8> TestAudioData;
};

namespace Inworld
{
	namespace Test
	{
		IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSendAudioMessageToCharacter, "Inworld.Conversation.SendAudioMessageToCharacter", Flags)
		bool FSendAudioMessageToCharacter::RunTest(const FString& Parameters)
		{
			TInworldTestObjectSessionScoped<UInworldTestObjectSendAudioMessageToCharacter> TestObject(this);

			ADD_LATENT_AUTOMATION_COMMAND(InitSpeechProcessor(TestObject->Session, EInworldPlayerSpeechMode::DEFAULT, {}))
			ADD_LATENT_AUTOMATION_COMMAND(SendCharacterAudioSessionStart(TestObject->Characters[0], FInworldAudioSessionOptions::Default()));
			const int MaxChunkSize = 3200;
			for (int32 i = 44; i < TestObject->TestAudioData.Num(); i += MaxChunkSize)
			{
				const int32 AudioChunkSize = FMath::Min(i + MaxChunkSize, TestObject->TestAudioData.Num());
				TArray<uint8> AudioChunk(TestObject->TestAudioData.GetData() + i, AudioChunkSize);
				ADD_LATENT_AUTOMATION_COMMAND(SendCharacterAudioData(TestObject->Characters[0], AudioChunk));
				ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(0.1f));
			}
			ADD_LATENT_AUTOMATION_COMMAND(SendCharacterAudioSessionStop(TestObject->Characters[0]));
			ADD_LATENT_AUTOMATION_COMMAND(DestroySpeechProcessor(TestObject->Session))

			ADD_LATENT_AUTOMATION_COMMAND(WaitUntilInteractionEnd(TestObject->ControlEvents));
			ADD_LATENT_AUTOMATION_COMMAND(TestTextEventCollectionNotEmpty(this, TestObject->TextEvents))
			ADD_LATENT_AUTOMATION_COMMAND(TestAudioDataEventCollectionNotEmpty(this, TestObject->AudioDataEvents))
			ADD_LATENT_AUTOMATION_COMMAND(TestTextEventCollectionValid(this, TestObject->TextEvents))
			ADD_LATENT_AUTOMATION_COMMAND(TestAudioDataEventCollectionValid(this, TestObject->AudioDataEvents))

			return true;
		}
	}
}
