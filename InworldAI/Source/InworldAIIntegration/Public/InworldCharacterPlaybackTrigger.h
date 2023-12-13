// Copyright 2023 Theai, Inc. (DBA Inworld) All Rights Reserved.

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

