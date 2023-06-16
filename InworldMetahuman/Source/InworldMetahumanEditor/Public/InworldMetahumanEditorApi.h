/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once


#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "InworldMetahumanEditorApi.generated.h"

UCLASS(BlueprintType, Config = Engine)
class INWORLDMETAHUMANEDITOR_API UInworldMetahumanEditorApi : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	virtual bool DoesSupportWorldType(EWorldType::Type WorldType) const override;

private:
	bool CanSetupAssetAsInworldMetahuman(const FAssetData& AssetData, bool bLogErrors = false);
	void SetupAssetAsInworldMetahuman(const FAssetData& AssetData);
};
