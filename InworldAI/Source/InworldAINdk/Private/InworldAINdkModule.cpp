/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldAINdkModule.h"

#define LOCTEXT_NAMESPACE "FInworldAINdkModule"

DEFINE_LOG_CATEGORY(LogInworld);

void FInworldAINdkModule::StartupModule()
{
}

void FInworldAINdkModule::ShutdownModule()
{
    
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FInworldAINdkModule, InworldAINdk)
