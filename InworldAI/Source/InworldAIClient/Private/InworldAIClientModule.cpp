/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldAIClientModule.h"

#define LOCTEXT_NAMESPACE "FInworldAIClientModule"

THIRD_PARTY_INCLUDES_START
#include "Utils/Log.h"
THIRD_PARTY_INCLUDES_END

DEFINE_LOG_CATEGORY(LogInworldAIClient);

DECLARE_LOG_CATEGORY_CLASS(LogInworldAINdk, Log, All);

void FInworldAIClientModule::StartupModule()
{
	Inworld::LogSetLoggerCallback([](const char* message, int severity)
		{
			switch (severity)
			{
			case 0:
				UE_LOG(LogInworldAINdk, Log, TEXT("%s"), UTF8_TO_TCHAR(message));
				break;
			case 1:
				UE_LOG(LogInworldAINdk, Warning, TEXT("%s"), UTF8_TO_TCHAR(message));
				break;
			case 2:
			{
				if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, UTF8_TO_TCHAR(message));
				UE_LOG(LogInworldAINdk, Error, TEXT("%s"), UTF8_TO_TCHAR(message));
				break;
			}
			default:
				UE_LOG(LogInworldAINdk, Warning, TEXT("Message with unknown severity, treating as warning: %s"), UTF8_TO_TCHAR(message));
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
