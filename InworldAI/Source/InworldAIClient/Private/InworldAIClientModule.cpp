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

struct FInworldLoggerUnreal : public Inworld::Logger
{
	virtual void Log(const std::string& message)
	{
		UE_LOG(LogInworldAINdk, Log, TEXT("%s"), UTF8_TO_TCHAR(message.c_str()));
	}

	virtual void LogWarning(const std::string& message)
	{
		UE_LOG(LogInworldAINdk, Warning, TEXT("%s"), UTF8_TO_TCHAR(message.c_str()));
	}

	virtual void LogError(const std::string& message)
	{
		UE_LOG(LogInworldAINdk, Error, TEXT("%s"), UTF8_TO_TCHAR(message.c_str()));
	}
};

void FInworldAIClientModule::StartupModule()
{
	Inworld::LogSetLogger<FInworldLoggerUnreal>();
}

void FInworldAIClientModule::ShutdownModule()
{
	Inworld::LogClearLogger();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FInworldAIClientModule, InworldAIClient)
