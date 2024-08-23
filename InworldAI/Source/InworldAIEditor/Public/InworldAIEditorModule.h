/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "ContentBrowserDelegates.h"
#include "Studio/InworldStudioTypes.h"
#include "UnrealEdMisc.h"
#include "Blueprint/UserWidget.h"
#include "Studio/InworldStudioWidget.h"

INWORLDAIEDITOR_API DECLARE_LOG_CATEGORY_EXTERN(LogInworldAIEditor, Log, All);

DECLARE_DELEGATE_OneParam(FAssetAction, const FAssetData&);
DECLARE_DELEGATE_RetVal_OneParam(bool, FAssetActionPermission, const FAssetData&);

class INWORLDAIEDITOR_API FInworldAIEditorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	void SetStudioWidgetState(const FInworldStudioWidgetState& Data)
	{
		StudioWidgetState = Data;
	}

	const FInworldStudioWidgetState& GetStudioWidgetState() const
	{
		return StudioWidgetState;
	}

private:
	TSharedRef<SDockTab> CreateInworldStudioTab(const FSpawnTabArgs& Args);
	TSharedRef<SWidget> CreateInworldStudioWidget();
	void OnLevelEditorMapChanged(UWorld* World, EMapChangeType MapChangeType);

	void AssetExtenderFunc(FMenuBuilder& MenuBuilder, const TArray<FAssetData> SelectedAssets);
	TSharedRef<FExtender> OnExtendAssetSelectionMenu(const TArray<FAssetData>& SelectedAssets);

	FInworldStudioWidgetState StudioWidgetState;

public:
	void BindMenuAssetAction(const FName& Name, const FName& Section, FText Label, FText Tooltip, FAssetAction Action, FAssetActionPermission ActionPermission);
	void UnbindMenuAssetAction(const FName& Name);

private:
	struct FAssetActionMenuFunction
	{
	public:
		FText Label;
		FText Tooltip;
		FSlateIcon Icon;
		FAssetAction Action;
		FAssetActionPermission ActionPermission;
	};

	struct FAssetActionMenuSection
	{
		TMap<FName, FAssetActionMenuFunction> FunctionMap;
	};

	struct FAssetActionMenu
	{
	public:
		TMap<FName, FAssetActionMenuSection> SectionMap;
		TMap<FName, FName> NameToSectionMap;
		TSet<FName> Sections;
	};

	FAssetActionMenu AssetActionMenu;

	TWeakPtr<SDockTab> InworldStudioTab;
	UUserWidget* InworldStudioUMGWidget;
};
