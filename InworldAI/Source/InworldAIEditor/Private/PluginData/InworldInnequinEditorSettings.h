/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "InworldCharacterComponent.h"
#include "InworldCharacterPlayback.h"
#include "InworldInnequinEditorSettings.generated.h"

UCLASS(config=InworldAI)
class INWORLDAIEDITOR_API UInworldInnequinEditorSettings : public UObject
{
	GENERATED_BODY()
public:
	UInworldInnequinEditorSettings(const FObjectInitializer& ObjectInitializer);

public:
	UPROPERTY(config, EditAnywhere, Category = "Innequin")
	TSoftObjectPtr<USkeletalMesh> InnequinMesh;

	UPROPERTY(config, EditAnywhere, Category = "Innequin")
	TSoftObjectPtr<UAnimBlueprint> InnequinABP;

	UPROPERTY(config, EditAnywhere, Category = "Innequin")
	TArray<TSubclassOf<UInworldCharacterPlayback>> CharacterPlaybacks;

	UPROPERTY(config, EditAnywhere, Category = "Innequin")
	TArray<TSubclassOf<UActorComponent>> CharacterComponents;
};
