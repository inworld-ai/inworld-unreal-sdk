// Copyright 2023 Theai, Inc. (DBA Inworld) All Rights Reserved.

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
