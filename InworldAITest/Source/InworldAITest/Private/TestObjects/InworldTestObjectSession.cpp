/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "TestObjects/InworldTestObjectSession.h"
#include "InworldAITestSettings.h"
#include "InworldAITestModule.h"

UInworldTestObjectSession::UInworldTestObjectSession()
	: UInworldTestObject()
	, Session(NewObject<UInworldSession>())
	, Player(NewObject<UInworldPlayer>())
{
	Session->Init();

	const UInworldAITestSettings* InworldAITestSettings = GetDefault<UInworldAITestSettings>();
	SceneName = InworldAITestSettings->SceneName;

	Player->SetSession(Session);

	for (const FString& CharacterName : InworldAITestSettings->InitialCharacterNames)
	{
		UInworldCharacter* const Character = Characters.Emplace_GetRef(NewObject<UInworldCharacter>());
		Character->SetBrainName(CharacterName);
		Character->SetSession(Session);
#define BIND_INWORLD_CHARACTER_EVENT(Type) \
		Character->OnInworld##Type##Event().AddLambda( [this, Character](const FInworld##Type##Event& Type##Event) { OnInworld##Type##Event_Internal(Character, Type##Event); } );

		BIND_INWORLD_CHARACTER_EVENT(Text)
		Character->OnInworldAudioEvent().AddLambda([this, Character](const FInworldAudioDataEvent& AudioDataEvent) { OnInworldAudioDataEvent_Internal(Character, AudioDataEvent); });
		BIND_INWORLD_CHARACTER_EVENT(Control)

#undef BIND_INWORLD_CHARACTER_EVENT
	}
}

#define DEFINE_HANDLE_INWORLD_CHARACTER_EVENT(Type) \
	void UInworldTestObjectSession::OnInworld##Type##Event_Internal(UInworldCharacter* Character, const FInworld##Type##Event& Type##Event) \
	{ \
		FString EventLogMessage; \
		Type##Event.AppendDebugString(EventLogMessage);\
		UE_LOG(LogInworldAITest, Log, TEXT("%s"), *EventLogMessage);\
		OnInworld##Type##Event(Character, Type##Event);\
	} \

DEFINE_HANDLE_INWORLD_CHARACTER_EVENT(Text)
DEFINE_HANDLE_INWORLD_CHARACTER_EVENT(AudioData)
DEFINE_HANDLE_INWORLD_CHARACTER_EVENT(Control)

#undef DEFINE_HANDLE_INWORLD_CHARACTER_EVENT
