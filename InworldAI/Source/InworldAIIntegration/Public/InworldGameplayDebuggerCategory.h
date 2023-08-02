/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#ifdef WITH_GAMEPLAY_DEBUGGER

#include "GameplayDebuggerCategory.h"

class FInworldGameplayDebuggerCategory : public FGameplayDebuggerCategory
{
public:
	FInworldGameplayDebuggerCategory();

	virtual void CollectData(APlayerController* OwnerPC, AActor* DebugActor) override;
	virtual void DrawData(APlayerController* OwnerPC, FGameplayDebuggerCanvasContext& CanvasContext) override;

	static TSharedRef<FGameplayDebuggerCategory> MakeInstance();

protected:
	struct FCharRepData
	{
		FString GivenName;
		FString AgentId;
		FVector OverheadLocation;
		bool bIsInteracting = false;

		FString CurrentMessage;

		int32 MessageQueueEntries = 0;

		uint8 EmotionalBehavior = 0;
		uint8 EmotionStrength = 0;

		bool bPendingRepAudioEvent = false;

		void Serialize(FArchive& Ar);
	};

	struct FPlayerRepData
	{
		FString TargetCharacterAgentId;
		bool bServerCapturingVoice = false;

		void Serialize(FArchive& Ar);
	};

	struct FRepData
	{
		TArray<FPlayerRepData> PlayerData;
		TArray<FCharRepData> CharData;

		bool bInworldApiExists = false;
		uint8 SessionStatus = 0;
		FString SessionId;
		FString SessionError;
		int32 ErrorCode;

		void Serialize(FArchive& Ar);
	};

	FRepData DataPack;
};

#endif // WITH_GAMEPLAY_DEBUGGER