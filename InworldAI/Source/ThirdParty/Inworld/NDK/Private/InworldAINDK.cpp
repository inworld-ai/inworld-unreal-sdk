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
	FString DllDirectory = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("InworldAI"))->GetBaseDir(), TEXT("Source/ThirdParty/Inworld/NDKLibrary/lib"));

	FString LibraryPath;
#if PLATFORM_WINDOWS
	LibraryPath = FPaths::Combine(*DllDirectory, TEXT("Win64/webrtc_aec_plugin.dll"));
#endif //PLATFORM_WINDOWS
#if PLATFORM_MAC
	LibraryPath = FPaths::Combine(*DllDirectory, TEXT("Mac/libwebrtc_aec_plugin.dylib"));
#endif //PLATFORM_MAC

#ifdef INWORLD_AEC
	webrtcLibraryHandle = !LibraryPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*LibraryPath) : nullptr;

	if (webrtcLibraryHandle == nullptr)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("InworldAINDKModuleError", "Failed to load webrtc library"));
	}
#endif //INWORLD_AEC

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

#ifdef INWORLD_AEC
	FPlatformProcess::FreeDllHandle(webrtcLibraryHandle);
#endif //INWORLD_AEC

	webrtcLibraryHandle = nullptr;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FInworldAINDKModule, InworldAINDK)
