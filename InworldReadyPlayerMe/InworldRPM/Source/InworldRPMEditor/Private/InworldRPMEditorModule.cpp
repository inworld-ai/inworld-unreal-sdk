/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldRPMEditorModule.h"
#include "InworldRPMEditorSettings.h"
#include "ISettingsModule.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FInworldRPMEditorModule"

DEFINE_LOG_CATEGORY(LogInworldRPMEditor);

void FInworldRPMEditorModule::StartupModule()
{
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
	if (SettingsModule)
	{
		SettingsModule->RegisterSettings("Project", "Plugins", "InworldAIRPMSettings",
			LOCTEXT("InworldRPMSettingsName", "InworldAI - RPM"), LOCTEXT("InworldRPMSettingsDescription", "Inworld AI Ready Player Me Settings"),
			GetMutableDefault<UInworldRPMEditorSettings>());
	}
}

void FInworldRPMEditorModule::ShutdownModule()
{
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
	if (SettingsModule)
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "InworldAIRPMSettings");
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FInworldRPMEditorModule, InworldRPMEditorModule)
