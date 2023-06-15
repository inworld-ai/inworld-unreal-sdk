/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldMetahumanEditorModule.h"
#include "InworldMetahumanEditorSettings.h"
#include "ISettingsModule.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FInworldMetahumanEditorModule"

void FInworldMetahumanEditorModule::StartupModule()
{
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
	if (SettingsModule)
	{
		SettingsModule->RegisterSettings("Project", "Plugins", "InworldAIMetahumanSettings",
			LOCTEXT("InworldRPMSettingsName", "InworldAI - Metahuman"), LOCTEXT("InworldMetahumanSettingsDescription", "Inworld AI Metahuman Settings"),
			GetMutableDefault<UInworldMetahumanEditorSettings>());
	}
}

void FInworldMetahumanEditorModule::ShutdownModule()
{
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
	if (SettingsModule)
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "InworldAIMetahumanSettings");
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FInworldMetahumanEditorModule, InworldMetahumanEditor)