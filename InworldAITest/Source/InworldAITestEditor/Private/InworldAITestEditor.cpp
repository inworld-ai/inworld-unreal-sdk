/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldAITestEditor.h"
#include "InworldAITestSettings.h"
#include "ISettingsModule.h"

#define LOCTEXT_NAMESPACE "FInworldAITestModule"

void FInworldAITestEditorModule::StartupModule()
{
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
	if (SettingsModule)
	{
		SettingsModule->RegisterSettings("Project", "Plugins", "InworldAITestSettings",
			LOCTEXT("InworldAITestSettingsName", "InworldAI - Test"), LOCTEXT("InworldAITestSettingsDescription", "Inworld AI Test Settings"),
			GetMutableDefault<UInworldAITestSettings>());
	}
}

void FInworldAITestEditorModule::ShutdownModule()
{
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
	if (SettingsModule)
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "InworldAITestSettings");
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FInworldAITestEditorModule, InworldAITestEditor)
