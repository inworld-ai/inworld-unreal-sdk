/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "Commands/InworldTestCommandsInteraction.h"
#include "InworldTestChecks.h"

bool Inworld::Test::FTestInteractionEndTrueCommand::Update()
{
	CheckTrue(TEXT("Interaction End"), ControlEvents.FilterByPredicate(
		[](const FInworldControlEvent& ControlEvent) -> bool
		{
			return ControlEvent.Action == EInworldControlEventAction::INTERACTION_END;
		}
	).Num() >= Count);
	return true;
}

bool Inworld::Test::FTestInteractionEndFalseCommand::Update()
{
	CheckFalse(TEXT("Interaction End"), ControlEvents.FilterByPredicate(
		[](const FInworldControlEvent& ControlEvent) -> bool
		{
			return ControlEvent.Action == EInworldControlEventAction::INTERACTION_END;
		}
	).Num() >= Count);
	return true;
}

bool Inworld::Test::FWaitUntilInteractionEndCommand::Update()
{
	return ControlEvents.FilterByPredicate(
		[](const FInworldControlEvent& ControlEvent) -> bool
		{
			return ControlEvent.Action == EInworldControlEventAction::INTERACTION_END;
		}
	).Num() >= Count;
}

#define DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_CHECK_EVENT_COLLECTION_EMPTY(Type) \
		bool Inworld::Test::FTest##Type##EventCollection##EmptyCommand::Update()\
		{\
			const FString TypeString = FString(TEXT(#Type)); \
			CheckTrue(*FString::Printf(TEXT("%sEvents is empty"), *TypeString), Type##Events.IsEmpty()); \
			return true;\
		}\
		bool Inworld::Test::FTest##Type##EventCollection##NotEmptyCommand::Update()\
		{\
			const FString TypeString = FString(TEXT(#Type)); \
			CheckFalse(*FString::Printf(TEXT("%sEvents is empty"), *TypeString), Type##Events.IsEmpty()); \
			return true;\
		}\

#define DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_CHECK_EVENT_COLLECTION_VALID(Type) \
		bool Inworld::Test::FTest##Type##EventCollection##ValidCommand::Update()\
		{\
			for(const auto& Type##Event : Type##Events)\
			{\
				const FString TypeString = FString(TEXT(#Type)); \
				CheckTrue(*FString::Printf(TEXT("%sEvent is valid"), *TypeString), Type##EventValid(Type##Event)); \
			}\
			return true;\
		}\

#define DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_TEST_EVENT_COLLECTION(Type) \
		void Inworld::Test::Test##Type##EventCollection(const TArray<FInworld##Type##Event>& Type##Events)\
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
