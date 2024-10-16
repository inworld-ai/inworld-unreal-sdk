/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldAITestModule.h"

#define LOCTEXT_NAMESPACE "FInworldAITestModule"

DEFINE_LOG_CATEGORY(LogInworldAITest);

void FInworldAITestModule::StartupModule()
{
}

void FInworldAITestModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FInworldAITestModule, InworldAITest)
