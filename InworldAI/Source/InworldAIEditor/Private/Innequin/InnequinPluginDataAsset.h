/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "InnequinPluginDataAsset.generated.h"

UCLASS()
class UInnequinPluginDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USkeletalMesh> SkeletalMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UAnimBlueprint> AnimBlueprint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TSubclassOf<UActorComponent> InnequinComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TSubclassOf<UActorComponent> EmoteComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<TSubclassOf<class UInworldCharacterPlayback>> CharacterPlaybacks;
};
