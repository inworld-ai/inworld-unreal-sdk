// Copyright Epic Games, Inc. All Rights Reserved.

#include "InworldAINDK.h"
#include "Core.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"

THIRD_PARTY_INCLUDES_START
#include "Utils/Log.h"
THIRD_PARTY_INCLUDES_END

#define LOCTEXT_NAMESPACE "FInworldNDKModule"

DECLARE_LOG_CATEGORY_CLASS(LogInworldAINDK, Log, All);

void FInworldAINDKModule::StartupModule()
{
	FString BaseDir = IPluginManager::Get().FindPlugin("InworldAI")->GetBaseDir();

	FString LibraryPath;
#if PLATFORM_WINDOWS
	LibraryPath = FPaths::Combine(*BaseDir, TEXT("Source/ThirdParty/Inworld/NDKLibrary/lib/Win64/webrtc_aec_plugin.dll"));
#elif PLATFORM_MAC
	LibraryPath = FPaths::Combine(*BaseDir, TEXT("Source/ThirdParty/Inworld/NDKLibrary/lib/Mac/libwebrtc_aec_plugin.dylib"));
#endif

#if INWORLD_AEC
	webrtcLibraryHandle = !LibraryPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*LibraryPath) : nullptr;

	if (webrtcLibraryHandle == nullptr)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("InworldAINDKModuleError", "Failed to load third party library"));
	}
#endif

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

void FInworldAINDKModule::ShutdownModule()
{
	Inworld::LogClearLoggerCallback();

#if INWORLD_AEC
	FPlatformProcess::FreeDllHandle(webrtcLibraryHandle);
#endif

	webrtcLibraryHandle = nullptr;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FInworldAINDKModule, InworldAINDK)
