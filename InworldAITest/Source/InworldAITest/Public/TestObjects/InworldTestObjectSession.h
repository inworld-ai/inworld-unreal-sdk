/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "TestObjects/InworldTestObject.h"
#include "InworldCharacter.h"
#include "InworldPlayer.h"
#include "InworldSession.h"
#include "InworldPackets.h"
#include "InworldTestObjectSession.generated.h"

UCLASS()
class UInworldTestObjectSession : public UInworldTestObject
{
	GENERATED_BODY()

public:
	UInworldTestObjectSession();

	UPROPERTY()
	FString SceneName;

	UPROPERTY()
	TObjectPtr<UInworldSession> Session;

	UPROPERTY()
	TObjectPtr<UInworldPlayer> Player;

	UPROPERTY()
	TArray<TObjectPtr<UInworldCharacter>> Characters;

#define DECLARE_HANDLE_INWORLD_CHARACTER_EVENT(Type) \
private: \
	void OnInworld##Type##Event_Internal(UInworldCharacter* Character, const FInworld##Type##Event& Type##Event); \
protected: \
	virtual void OnInworld##Type##Event(UInworldCharacter* Character, const FInworld##Type##Event& Type##Event) { Type##Events.Add(Type##Event); } \
public: \
	TArray<FInworld##Type##Event> Type##Events; \

	DECLARE_HANDLE_INWORLD_CHARACTER_EVENT(Text)
	DECLARE_HANDLE_INWORLD_CHARACTER_EVENT(AudioData)
	DECLARE_HANDLE_INWORLD_CHARACTER_EVENT(Control)

#undef DECLARE_HANDLE_INWORLD_CHARACTER_EVENT
};
