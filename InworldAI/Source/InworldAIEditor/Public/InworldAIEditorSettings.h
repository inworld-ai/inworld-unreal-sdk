// Copyright 2023 Theai, Inc. (DBA Inworld) All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "InworldPlayerComponent.h"
#include "InworldCharacterComponent.h"
#include "InworldCharacterPlayback.h"
#include "InworldAIEditorSettings.generated.h"

UCLASS(config=InworldAI)
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

public:
	UPROPERTY(config, VisibleAnywhere, Category = "Studio")
	FSoftObjectPath InworldStudioWidget;
};
