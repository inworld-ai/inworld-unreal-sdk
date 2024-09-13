/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "TestObjects/InworldTestObject.h"
#include "InworldAITestModule.h"
#include "Commands/InworldTestCommandsSession.h"
#include "InworldEnums.h"
#include "InworldTypes.h"
#include "InworldCharacter.h"
#include "InworldPlayer.h"
#include "InworldSession.h"
#include "InworldTestObjectSession.generated.h"

UCLASS()
class UInworldTestObjectSession : public UInworldTestObject
{
	GENERATED_BODY()

public:
	UInworldTestObjectSession()
		: UInworldTestObject()
		, Session(NewObject<UInworldSession>())
		, Player(NewObject<UInworldPlayer>())
	{
		Session->Init();

		const UInworldAITestSettings* InworldAITestSettings = GetDefault<UInworldAITestSettings>();
		SceneName = InworldAITestSettings->SceneName;

		for (const FString& CharacterName : InworldAITestSettings->CharacterNames)
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

	UPROPERTY()
	FString SceneName;

	UPROPERTY()
	TObjectPtr<UInworldSession> Session;

	UPROPERTY()
	TObjectPtr<UInworldPlayer> Player;

	UPROPERTY()
	TArray<TObjectPtr<UInworldCharacter>> Characters;

#define HANDLE_INWORLD_CHARACTER_EVENT(Type) \
private: \
	void OnInworld##Type##Event_Internal(UInworldCharacter* Character, const FInworld##Type##Event& Type##Event) \
	{ \
		FString EventLogMessage; \
		Type##Event.AppendDebugString(EventLogMessage);\
		UE_LOG(LogInworldAITest, Log, TEXT("%s"), *EventLogMessage);\
		OnInworld##Type##Event(Character, Type##Event);\
	} \
protected: \
	virtual void OnInworld##Type##Event(UInworldCharacter* Character, const FInworld##Type##Event& Type##Event) { Type##Events.Add(Type##Event); } \
public: \
	TArray<FInworld##Type##Event> Type##Events; \

	HANDLE_INWORLD_CHARACTER_EVENT(Text)
	HANDLE_INWORLD_CHARACTER_EVENT(AudioData)
	HANDLE_INWORLD_CHARACTER_EVENT(Control)

#undef HANDLE_INWORLD_CHARACTER_EVENT
};
