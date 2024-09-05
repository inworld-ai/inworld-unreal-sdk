/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldAILLMModule.h"

#define LOCTEXT_NAMESPACE "FInworldAILLMModule"

DEFINE_LOG_CATEGORY(LogInworldAILLM);

void FInworldAILLMModule::StartupModule()
{
}

void FInworldAILLMModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FInworldAILLMModule, InworldAILLM)
