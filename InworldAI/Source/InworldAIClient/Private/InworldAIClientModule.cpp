/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldAIClientModule.h"
#ifdef INWORLD_WITH_NDK
THIRD_PARTY_INCLUDES_START
#include "Utils/Log.h"
THIRD_PARTY_INCLUDES_END
#endif
#define LOCTEXT_NAMESPACE "FInworldAIClientModule"

DEFINE_LOG_CATEGORY(LogInworldAIClient);

DEFINE_LOG_CATEGORY(LogInworldAINDK);

void FInworldAIClientModule::StartupModule()
{
#ifdef INWORLD_WITH_NDK
	Inworld::SetLogCallbacks(
		[](const char* message)
		{
			UE_LOG(LogInworldAINDK, Log, TEXT("%s"), UTF8_TO_TCHAR(message));
		},
		[](const char* message)
		{
			UE_LOG(LogInworldAINDK, Warning, TEXT("%s"), UTF8_TO_TCHAR(message));
		},
		[](const char* message)
		{
			UE_LOG(LogInworldAINDK, Error, TEXT("%s"), UTF8_TO_TCHAR(message));
		}
	);
#endif
}

void FInworldAIClientModule::ShutdownModule()
{
#ifdef INWORLD_WITH_NDK
	Inworld::ClearLogCallbacks();
#endif
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FInworldAIClientModule, InworldAIClient)
