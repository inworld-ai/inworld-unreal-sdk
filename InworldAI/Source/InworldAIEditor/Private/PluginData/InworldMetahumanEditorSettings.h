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
#include "InworldMetahumanEditorSettings.generated.h"

UCLASS(config=InworldAI)
class INWORLDAIEDITOR_API UInworldMetahumanEditorSettings : public UObject
{
	GENERATED_BODY()
public:
	UInworldMetahumanEditorSettings(const FObjectInitializer& ObjectInitializer);

public:
	UPROPERTY(config, EditAnywhere, Category = "Metahuman")
	TSoftObjectPtr<USkeleton> MetahumanBodySkeleton;

	UPROPERTY(config, EditAnywhere, Category = "Metahuman")
	TSoftObjectPtr<UAnimBlueprint> MetahumanBodyABP;

	UPROPERTY(config, EditAnywhere, Category = "Metahuman")
	TSoftObjectPtr<USkeleton> MetahumanFaceSkeleton;

	UPROPERTY(config, EditAnywhere, Category = "Metahuman")
	TSoftObjectPtr<UAnimBlueprint> MetahumanFaceABP;

	UPROPERTY(config, EditAnywhere, Category = "Metahuman")
	TArray<TSubclassOf<UInworldCharacterPlayback>> CharacterPlaybacks;

	UPROPERTY(config, EditAnywhere, Category = "Metahuman")
	TArray<TSubclassOf<UActorComponent>> CharacterComponents;
};
