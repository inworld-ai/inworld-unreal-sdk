/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldAIIntegrationModule.h"
#include "NDK/Proto/ProtoDisableWarning.h"

#if defined(WITH_GAMEPLAY_DEBUGGER) && defined(INWORLD_DEBUGGER_SLOT)
#include "GameplayDebugger.h"
#include "InworldGameplayDebuggerCategory.h"
#endif // WITH_GAMEPLAY_DEBUGGER

#define LOCTEXT_NAMESPACE "FInworldAIIntegrationModule"

void FInworldAIIntegrationModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
#if defined(WITH_GAMEPLAY_DEBUGGER) && defined(INWORLD_DEBUGGER_SLOT)
	IGameplayDebugger& GameplayDebuggerModule = IGameplayDebugger::Get();
	GameplayDebuggerModule.RegisterCategory("Inworld", IGameplayDebugger::FOnGetCategory::CreateStatic(&FInworldGameplayDebuggerCategory::MakeInstance), EGameplayDebuggerCategoryState::Disabled, INWORLD_DEBUGGER_SLOT);
	GameplayDebuggerModule.NotifyCategoriesChanged();
#endif
}

void FInworldAIIntegrationModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
#if defined(WITH_GAMEPLAY_DEBUGGER) && defined(INWORLD_DEBUGGER_SLOT)
	if (IGameplayDebugger::IsAvailable())
	{
		IGameplayDebugger& GameplayDebuggerModule = IGameplayDebugger::Get();
		GameplayDebuggerModule.UnregisterCategory("Inworld");
		GameplayDebuggerModule.NotifyCategoriesChanged();
	}
#endif
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FInworldAIIntegrationModule, InworldAIIntegration)