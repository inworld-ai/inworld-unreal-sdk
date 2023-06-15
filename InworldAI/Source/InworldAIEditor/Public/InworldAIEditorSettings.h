/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "InworldPlayerComponent.h"
#include "InworldCharacterComponent.h"
#include "InworldCharacterPlayback.h"
#include "InworldAIEditorSettings.generated.h"

UCLASS(config=Engine)
class INWORLDAIEDITOR_API UInworldAIEditorSettings : public UObject
{
	GENERATED_BODY()
public:
	UInworldAIEditorSettings(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(config, EditAnywhere, Category = "Inworld")
	FString StudioAccessToken;

public:
	UPROPERTY(config, EditAnywhere, Category = "Player")
	TSubclassOf<UInworldPlayerComponent> InworldPlayerComponent;

	UPROPERTY(config, EditAnywhere, Category = "Player")
	TArray<TSubclassOf<UActorComponent>> OtherPlayerComponents;

public:
	UPROPERTY(config, EditAnywhere, Category = "Character")
	TSubclassOf<UInworldCharacterComponent> InworldCharacterComponent;

	UPROPERTY(EditAnywhere, Category = "Character")
	TArray<TSubclassOf<UInworldCharacterPlayback>> CharacterPlaybacks;

	UPROPERTY(EditAnywhere, Category = "Character")
	TArray<TSubclassOf<UActorComponent>> OtherCharacterComponents;
};
