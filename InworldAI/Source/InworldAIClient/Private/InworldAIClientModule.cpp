/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldAIClientModule.h"

#define LOCTEXT_NAMESPACE "FInworldAIClientModule"

DEFINE_LOG_CATEGORY(LogInworldAIClient);

void FInworldAIClientModule::StartupModule()
{
	
}

void FInworldAIClientModule::ShutdownModule()
{

}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FInworldAIClientModule, InworldAIClient)
