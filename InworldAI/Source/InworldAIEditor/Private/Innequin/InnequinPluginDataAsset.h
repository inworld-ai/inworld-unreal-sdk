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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Innequin")
	TSoftObjectPtr<USkeletalMesh> SkeletalMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Innequin")
	TSoftObjectPtr<UAnimBlueprint> AnimBlueprint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Innequin")
	TSubclassOf<UActorComponent> InnequinComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Innequin")
	TSubclassOf<UActorComponent> EmoteComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Innequin")
	TArray<TSubclassOf<class UInworldCharacterPlayback>> CharacterPlaybacks;
};
