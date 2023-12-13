// Copyright 2023 Theai, Inc. (DBA Inworld) All Rights Reserved.

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
				UE_LOG(LogInworldAINdk, Error, TEXT("%s"), UTF8_TO_TCHAR(message));
				break;
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
