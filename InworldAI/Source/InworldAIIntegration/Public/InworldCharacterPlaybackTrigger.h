/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "InworldCharacterPlayback.h"

#include "InworldCharacterPlaybackTrigger.generated.h"

UCLASS(BlueprintType, Blueprintable)
class INWORLDAIINTEGRATION_API UInworldCharacterPlaybackTrigger : public UInworldCharacterPlayback
{
	GENERATED_BODY()

protected:
	virtual void OnCharacterTrigger_Implementation(const FCharacterMessageTrigger& Message) override;
	virtual void OnCharacterInteractionEnd_Implementation(const FCharacterMessageInteractionEnd& Message) override;

private:
	TMap<FString, TArray<FCharacterMessageTrigger>> PendingTriggers;
};

