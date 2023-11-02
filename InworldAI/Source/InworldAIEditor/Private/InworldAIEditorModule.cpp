/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldAIEditorModule.h"
#include "IAssetTools.h"
#include "ContentBrowserModule.h"
#include "InworldEditorApi.h"
#include "InworldAIEditorSettings.h"
#include "InworldEditorUIStyle.h"
#include "ISettingsModule.h"
#include "WidgetBlueprint.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Docking/SDockTab.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"

#define LOCTEXT_NAMESPACE "FInworldAIEditorModule"

DEFINE_LOG_CATEGORY(LogInworldAIEditor);

void FInworldAIEditorModule::StartupModule()
{
	FInworldEditorUIStyle::Initialize();

	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	TArray<FContentBrowserMenuExtender_SelectedAssets>& CBMenuAssetExtenderDelegates = ContentBrowserModule.GetAllAssetViewContextMenuExtenders();
	CBMenuAssetExtenderDelegates.Add(FContentBrowserMenuExtender_SelectedAssets::CreateRaw(this, &FInworldAIEditorModule::OnExtendAssetSelectionMenu));

	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
	if (SettingsModule)
	{
		SettingsModule->RegisterSettings("Project", "Plugins", "InworldAIEditorSettings",
			LOCTEXT("InworldSettingsName", "InworldAI"), LOCTEXT("InworldSettingsDescription", "Inworld AI Editor Settings"),
			GetMutableDefault<UInworldAIEditorSettings>());
	}
	if (FSlateApplication::IsInitialized())
	{
		FGlobalTabmanager::Get()->RegisterNomadTabSpawner("Inworld Studio", FOnSpawnTab::CreateRaw(this, &FInworldAIEditorModule::CreateInworldStudioTab))
			.SetDisplayName(NSLOCTEXT("InworldStudio", "TabTitle", "Inworld Studio"))
			.SetTooltipText(NSLOCTEXT("InworldStudio", "TooltipText", "Tools for working with Inworld AI."))
			.SetGroup(WorkspaceMenu::GetMenuStructure().GetToolsCategory())
			.SetIcon(FSlateIcon(FInworldEditorUIStyle::Get()->GetStyleSetName(), "InworldEditor.Icon"));
	}
}

void FInworldAIEditorModule::ShutdownModule()
{
	FInworldEditorUIStyle::Shutdown();

	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
	if (SettingsModule)
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "InworldAIEditorSettings");
	}

	if (FSlateApplication::IsInitialized())
	{
		FGlobalTabmanager::Get()->UnregisterNomadTabSpawner("Inworld Studio");
	}
}

TSharedRef<SDockTab> FInworldAIEditorModule::CreateInworldStudioTab(const FSpawnTabArgs& Args)
{
	const TSharedRef<SDockTab> DockTab = SNew(SDockTab).TabRole(ETabRole::NomadTab);

	if (UWorld* EditorWorld = GEditor->GetEditorWorldContext().World())
	{
		const UInworldAIEditorSettings* InworldAIEditorSettings = GetDefault<UInworldAIEditorSettings>();

		TSoftObjectPtr<UWidgetBlueprint> InworldStudioWidget(InworldAIEditorSettings->InworldStudioWidget);
		InworldStudioWidget.LoadSynchronous();

		UUserWidget* Widget = CreateWidget<UEditorUtilityWidget>(EditorWorld, Cast<UClass>(InworldStudioWidget.Get()->GeneratedClass));
		if (Widget)
		{
			DockTab->SetContent(Widget->TakeWidget());
		}
	}

	return DockTab;
}

void FInworldAIEditorModule::AssetExtenderFunc(FMenuBuilder& MenuBuilder, const TArray<FAssetData> SelectedAssets)
{
	for (auto& Asset : SelectedAssets)
	{
		UBlueprint* const Blueprint = Cast<UBlueprint>(Asset.GetAsset());
		if (!Blueprint || !Blueprint->SimpleConstructionScript)
		{
			return;
		}
	}

	MenuBuilder.BeginSection("Inworld", LOCTEXT("ASSET_CONTEXT", "Inworld"));

	MenuBuilder.AddSubMenu(
		FText::FromString("Inworld Actions"),
		FText::FromString("Quick Inworld blueprint setup helpers"),
		FNewMenuDelegate::CreateLambda([this, SelectedAssets](FMenuBuilder& SubMenuBuilder)
			{
				for (const FName& SectionName : AssetActionMenu.Sections)
				{
					SubMenuBuilder.BeginSection(SectionName, FText::FromName(SectionName));
					auto& SectionFunctionMap = AssetActionMenu.SectionMap[SectionName].FunctionMap;
					for (TPair<FName, FAssetActionMenuFunction>& Entry : SectionFunctionMap)
					{
						FAssetActionMenuFunction& AssetActionMenuFunction = Entry.Value;
						SubMenuBuilder.AddMenuEntry(
							AssetActionMenuFunction.Label,
							AssetActionMenuFunction.Tooltip,
							AssetActionMenuFunction.Icon,
							FUIAction(
								FExecuteAction::CreateLambda([=]()
									{
										for (const auto& Asset : SelectedAssets)
										{
											AssetActionMenuFunction.Action.Execute(Asset);
										}
									}
								),
								FCanExecuteAction::CreateLambda([=]() -> bool
									{
										for (const auto& Asset : SelectedAssets)
										{
											if (!AssetActionMenuFunction.ActionPermission.Execute(Asset))
											{
												return false;
											}
										}
										return true;
									}
								)
							)
						);
					}
					SubMenuBuilder.EndSection();
				}
			}
		),
		false,
		FSlateIcon(FInworldEditorUIStyle::Get()->GetStyleSetName(), "InworldEditor.Icon")
	);
	
	MenuBuilder.EndSection();
}

TSharedRef<FExtender> FInworldAIEditorModule::OnExtendAssetSelectionMenu(const TArray<FAssetData>& SelectedAssets)
{
	TSharedRef<FExtender> Extender = MakeShared<FExtender>();
	Extender->AddMenuExtension(
		"CommonAssetActions",
		EExtensionHook::After,
		nullptr,
		FMenuExtensionDelegate::CreateRaw(this, &FInworldAIEditorModule::AssetExtenderFunc, SelectedAssets)
	);
	return Extender;
}

void FInworldAIEditorModule::SetupAssetAsInworldPlayer(const FAssetData& AssetData)
{
	if (auto* World = GEditor->GetEditorWorldContext().World())
	{
		World->GetSubsystem<UInworldEditorApiSubsystem>()->SetupAssetAsInworldPlayer(AssetData);
	}
}

bool FInworldAIEditorModule::CanSetupAssetAsInworldPlayer(const FAssetData& AssetData)
{
	if (auto* World = GEditor->GetEditorWorldContext().World())
	{
		return World->GetSubsystem<UInworldEditorApiSubsystem>()->CanSetupAssetAsInworldPlayer(AssetData);
	}
	return false;
}

void FInworldAIEditorModule::BindMenuAssetAction(const FName& Name, const FName& Section, FText Label, FText Tooltip, FAssetAction Action, FAssetActionPermission ActionPermission)
{
	UnbindMenuAssetAction(Name);

	if (!AssetActionMenu.SectionMap.Contains(Section))
	{
		AssetActionMenu.Sections.Add(Section);
		AssetActionMenu.SectionMap.Add(Section, FAssetActionMenuSection());
	}

	AssetActionMenu.NameToSectionMap.Add(Name, Section);

	FAssetActionMenuFunction AssetActionMenuFunction;
	AssetActionMenuFunction.Label = Label;
	AssetActionMenuFunction.Tooltip = Tooltip;
	AssetActionMenuFunction.Action = Action;
	AssetActionMenuFunction.ActionPermission = ActionPermission;

	AssetActionMenu.SectionMap[Section].FunctionMap.Add(Name, AssetActionMenuFunction);
}

void FInworldAIEditorModule::UnbindMenuAssetAction(const FName& Name)
{
	if (AssetActionMenu.NameToSectionMap.Contains(Name))
	{
		const FName& Section = AssetActionMenu.NameToSectionMap[Name];
		auto& FunctionMap = AssetActionMenu.SectionMap[Section].FunctionMap;
		FunctionMap.Remove(Name);
		if (FunctionMap.Num() == 0)
		{
			AssetActionMenu.Sections.Remove(Section);
			AssetActionMenu.SectionMap.Remove(Section);
		}
	}
	AssetActionMenu.NameToSectionMap.Remove(Name);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FInworldAIEditorModule, InworldAIEditor)