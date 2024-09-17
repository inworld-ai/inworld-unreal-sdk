/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "InworldTestMacros.h"
#include "InworldTestChecks.h"

#include "InworldSession.h"
#include "InworldCharacter.h"
#include "InworldPackets.h"

namespace Inworld
{
	namespace Test
	{
		DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(TestInteractionEndTrue, const TArray<FInworldControlEvent>&, ControlEvents);
		bool FTestInteractionEndTrueCommand::Update()
		{
			CheckTrue(TEXT("Interaction End"), nullptr != ControlEvents.FindByPredicate(
				[](const FInworldControlEvent& ControlEvent) -> bool
				{
					return ControlEvent.Action == EInworldControlEventAction::INTERACTION_END;
				}
			));
			return true;
		}

		DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(TestInteractionEndFalse, const TArray<FInworldControlEvent>&, ControlEvents);
		bool FTestInteractionEndFalseCommand::Update()
		{
			CheckFalse(TEXT("Interaction End"), nullptr != ControlEvents.FindByPredicate(
				[](const FInworldControlEvent& ControlEvent) -> bool
				{
					return ControlEvent.Action == EInworldControlEventAction::INTERACTION_END;
				}
			));
			return true;
		}

		DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(WaitUntilInteractionEnd, const TArray<FInworldControlEvent>&, ControlEvents);
		bool FWaitUntilInteractionEndCommand::Update()
		{
			return nullptr != ControlEvents.FindByPredicate(
				[](const FInworldControlEvent& ControlEvent) -> bool
				{
					return ControlEvent.Action == EInworldControlEventAction::INTERACTION_END;
				}
			);
		}

#define DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_CHECK_EVENT_COLLECTION_EMPTY(Type) \
		DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(Test##Type##EventCollection##Empty, const TArray<FInworld##Type##Event>&, Type##Events);\
		bool FTest##Type##EventCollection##EmptyCommand::Update()\
		{\
			const FString TypeString = FString(TEXT(#Type)); \
			CheckTrue(*FString::Printf(TEXT("%sEvents is empty"), *TypeString), Type##Events.IsEmpty()); \
			return true;\
		}\
		DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(Test##Type##EventCollection##NotEmpty, const TArray<FInworld##Type##Event>&, Type##Events);\
		bool FTest##Type##EventCollection##NotEmptyCommand::Update()\
		{\
			const FString TypeString = FString(TEXT(#Type)); \
			CheckFalse(*FString::Printf(TEXT("%sEvents is empty"), *TypeString), Type##Events.IsEmpty()); \
			return true;\
		}\

#define DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_CHECK_EVENT_COLLECTION_VALID(Type) \
		DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(Test##Type##EventCollection##Valid, const TArray<FInworld##Type##Event>&, Type##Events);\
		bool FTest##Type##EventCollection##ValidCommand::Update()\
		{\
			for(const auto& Type##Event : Type##Events)\
			{\
				const FString TypeString = FString(TEXT(#Type)); \
				CheckTrue(*FString::Printf(TEXT("%sEvent is valid"), *TypeString), Type##EventValid(Type##Event)); \
			}\
			return true;\
		}\

#define DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_TEST_EVENT_COLLECTION(Type) \
		void Test##Type##EventCollection(const TArray<FInworld##Type##Event>& Type##Events)\
		{\
			Test##Type##EventCollectionNotEmpty(Type##Events); \
			Test##Type##EventCollectionValid(Type##Events); \
		}\

		TFunction<bool(const FInworldTextEvent&)> TextEventValid = [](const FInworldTextEvent& TextEvent) -> bool
		{
			return !TextEvent.Text.IsEmpty();
		};
		DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_CHECK_EVENT_COLLECTION_EMPTY(Text)
		DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_CHECK_EVENT_COLLECTION_VALID(Text)
		DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_TEST_EVENT_COLLECTION(Text)

		TFunction<bool(const FInworldAudioDataEvent&)> AudioDataEventValid = [](const FInworldAudioDataEvent& AudioDataEvent) -> bool
		{
			return !AudioDataEvent.Chunk.IsEmpty();
		};
		DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_CHECK_EVENT_COLLECTION_EMPTY(AudioData)
		DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_CHECK_EVENT_COLLECTION_VALID(AudioData)
		DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_TEST_EVENT_COLLECTION(AudioData)

#undef DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_CHECK_EVENT_COLLECTION_EMPTY
#undef DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_CHECK_EVENT_COLLECTION_VALID
#undef DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_TEST_EVENT_COLLECTION
	}
}
