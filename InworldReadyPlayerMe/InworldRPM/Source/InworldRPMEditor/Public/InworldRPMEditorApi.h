/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "InworldStudioTypes.h"
#include "InworldRPMEditorApi.generated.h"

UCLASS()
class INWORLDRPMEDITOR_API UInworldRPMEditorApi : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	virtual bool DoesSupportWorldType(EWorldType::Type WorldType) const override;

	UFUNCTION(BlueprintCallable, Category = "InworldAI")
	void CreateReadyPlayerMeActor(const FInworldStudioUserCharacterData& CharacterData);

	UFUNCTION(BlueprintCallable, Category = "InworldAI")
	bool CanCreateReadyPlayerMeActor(const FInworldStudioUserCharacterData& CharacterData);

private:
	UPROPERTY()
	class UglTFRuntimeAsset* glTFRuntimeAsset = nullptr;
	UPROPERTY()
	class USkeletalMesh* RuntimeSkeletalMesh = nullptr;
	UPROPERTY()
	class UBlueprint* RuntimeActorBP = nullptr;
};
