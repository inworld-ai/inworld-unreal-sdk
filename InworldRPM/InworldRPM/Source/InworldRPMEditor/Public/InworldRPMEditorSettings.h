/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "InworldCharacterPlayback.h"
#include "InworldRPMEditorSettings.generated.h"

UCLASS(Config=InworldAI)
class INWORLDRPMEDITOR_API UInworldRPMEditorSettings : public UObject
{
	GENERATED_BODY()

public:
	UInworldRPMEditorSettings(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(config, EditAnywhere, Category = "Ready Player Me")
	FSoftObjectPath RPMAnimBP;

	UPROPERTY(config, EditAnywhere, Category = "Ready Player Me")
	FSoftObjectPath RPMSkeleton;

	UPROPERTY(config, EditAnywhere, Category = "Ready Player Me")
	TSubclassOf<UInworldCharacterComponent> InworldCharacterComponent;

	UPROPERTY(EditAnywhere, Category = "Ready Player Me")
	TArray<TSubclassOf<UInworldCharacterPlayback>> CharacterPlaybacks;

	UPROPERTY(EditAnywhere, Category = "Ready Player Me")
	TArray<TSubclassOf<UActorComponent>> OtherCharacterComponents;
};
