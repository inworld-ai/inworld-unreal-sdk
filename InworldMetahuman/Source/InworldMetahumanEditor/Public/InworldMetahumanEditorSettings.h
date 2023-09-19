/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
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
class INWORLDMETAHUMANEDITOR_API UInworldMetahumanEditorSettings : public UObject
{
	GENERATED_BODY()
public:
	UInworldMetahumanEditorSettings(const FObjectInitializer& ObjectInitializer);

public:
	UPROPERTY(config, EditAnywhere, Category = "Metahuman")
	FSoftObjectPath MetahumanAnimBP;

	UPROPERTY(config, EditAnywhere, Category = "Metahuman")
	FSoftObjectPath MetahumanBody;

	UPROPERTY(config, EditAnywhere, Category = "Metahuman")
	FSoftObjectPath MetahumanFaceAnimBP;

	UPROPERTY(config, EditAnywhere, Category = "Metahuman")
	FSoftObjectPath MetahumanFace;

	UPROPERTY(config, EditAnywhere, Category = "Metahuman")
	TSubclassOf<UInworldCharacterComponent> InworldCharacterComponent;

	UPROPERTY(EditAnywhere, Category = "Metahuman")
	TArray<TSubclassOf<UInworldCharacterPlayback>> CharacterPlaybacks;

	UPROPERTY(EditAnywhere, Category = "Metahuman")
	TArray<TSubclassOf<UActorComponent>> OtherCharacterComponents;
};
