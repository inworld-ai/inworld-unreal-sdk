/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldAIEditorModule.h"
#include "AssetTools/Public/IAssetTools.h"
#include "ContentBrowserModule.h"
#include "InworldEditorApi.h"
#include "InworldAIEditorSettings.h"
#include "InworldEditorUIStyle.h"
#include "ISettingsModule.h"
#include "WidgetBlueprint.h"
#include "Blutility/Classes/EditorUtilityWidget.h"
#include "Blutility/Classes/EditorUtilityWidgetBlueprint.h"
#include "Blutility/Public/EditorUtilitySubsystem.h"
#include "MainFrame/Public/Interfaces/IMainFrameModule.h"

#define LOCTEXT_NAMESPACE "FInworldAIEditorModule"

DEFINE_LOG_CATEGORY(LogInworldAIEditor);

void FInworldAIEditorModule::StartupModule()
{
	FInworldEditorUIStyle::Initialize();

	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	TArray<FContentBrowserMenuExtender_SelectedAssets>& CBMenuAssetExtenderDelegates = ContentBrowserModule.GetAllAssetViewContextMenuExtenders();
	CBMenuAssetExtenderDelegates.Add(FContentBrowserMenuExtender_SelectedAssets::CreateRaw(this, &FInworldAIEditorModule::OnExtendAssetSelectionMenu));

	IMainFrameModule& MainFrameModule = IMainFrameModule::Get();
	OpenInworldStudioWidgetDelHandle = MainFrameModule.OnMainFrameCreationFinished().AddRaw(this, &FInworldAIEditorModule::OpenInworldStudioWidget);

	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
	if (SettingsModule)
	{
		SettingsModule->RegisterSettings("Project", "Plugins", "InworldAIEditorSettings",
			LOCTEXT("InworldSettingsName", "InworldAI"), LOCTEXT("InworldSettingsDescription", "Inworld AI Editor Settings"),
			GetMutableDefault<UInworldAIEditorSettings>());
	}
}

void FInworldAIEditorModule::ShutdownModule()
{
	FInworldEditorUIStyle::Shutdown();

	IMainFrameModule& MainFrameModule = IMainFrameModule::Get();
	MainFrameModule.OnMainFrameCreationFinished().Remove(OpenInworldStudioWidgetDelHandle);

	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
	if (SettingsModule)
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "InworldAIEditorSettings");
	}
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
		FNewMenuDelegate::CreateLambda([=](FMenuBuilder& SubMenuBuilder)
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

void FInworldAIEditorModule::OpenInworldStudioWidget(TSharedPtr<SWindow>, bool)
{
	FSoftObjectPath StudioWidgetPath(TEXT("/InworldAI/StudioWidget/InworldStudioWidget.InworldStudioWidget"));
	TSoftObjectPtr<UObject> Asset(StudioWidgetPath);

	if (UWidgetBlueprint* Blueprint = Cast<UWidgetBlueprint>(Asset.LoadSynchronous()))
	{
		if (Blueprint->GeneratedClass->IsChildOf(UEditorUtilityWidget::StaticClass()))
		{
			UEditorUtilityWidgetBlueprint* EditorWidget = Cast<UEditorUtilityWidgetBlueprint>(Blueprint);
			if (EditorWidget)
			{
				UEditorUtilitySubsystem* EditorUtilitySubsystem = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>();
				FName ID;
				EditorUtilitySubsystem->RegisterTabAndGetID(EditorWidget, ID);
			}
		}
	}
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