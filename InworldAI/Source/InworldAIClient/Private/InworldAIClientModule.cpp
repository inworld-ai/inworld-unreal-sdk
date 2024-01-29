/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldAIClientModule.h"

THIRD_PARTY_INCLUDES_START
#include "Utils/Log.h"
THIRD_PARTY_INCLUDES_END

#define LOCTEXT_NAMESPACE "FInworldAIClientModule"

DEFINE_LOG_CATEGORY(LogInworldAIClient);

DEFINE_LOG_CATEGORY(LogInworldAINDK);

void FInworldAIClientModule::StartupModule()
{
	Inworld::LogSetLoggerCallback([](const char* message, int severity)
		{
			switch (severity)
			{
			case 0:
				UE_LOG(LogInworldAINDK, Log, TEXT("%s"), UTF8_TO_TCHAR(message));
				break;
			case 1:
				UE_LOG(LogInworldAINDK, Warning, TEXT("%s"), UTF8_TO_TCHAR(message));
				break;
			case 2:
				UE_LOG(LogInworldAINDK, Error, TEXT("%s"), UTF8_TO_TCHAR(message));
				break;
			default:
				UE_LOG(LogInworldAINDK, Warning, TEXT("Message with unknown severity, treating as warning: %s"), UTF8_TO_TCHAR(message));
			}
		}
	);
}

void FInworldAIClientModule::ShutdownModule()
{
	Inworld::LogClearLoggerCallback();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FInworldAIClientModule, InworldAIClient)
