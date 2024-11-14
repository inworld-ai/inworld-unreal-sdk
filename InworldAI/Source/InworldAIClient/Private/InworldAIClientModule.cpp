/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldAIClientModule.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"
#include "HAL/PlatformProcess.h"
#include "Misc/Paths.h"

#ifdef INWORLD_WITH_NDK
THIRD_PARTY_INCLUDES_START
#include "Utils/Log.h"
THIRD_PARTY_INCLUDES_END
#endif

#if WITH_EDITOR
#include "Misc/MessageDialog.h"
#endif //WITH_EDITOR

#define LOCTEXT_NAMESPACE "FInworldAIClientModule"

DEFINE_LOG_CATEGORY(LogInworldAIClient);

#ifdef INWORLD_WITH_NDK
DEFINE_LOG_CATEGORY_STATIC(LogInworldAINDK, Log, All);
#endif

#ifdef INWORLD_WITH_NDK
static void LoadDll(const FString& Path, void** Handle)
{
	*Handle = !Path.IsEmpty() ? FPlatformProcess::GetDllHandle(*Path) : nullptr;
#if WITH_EDITOR
	if (Handle == nullptr)
	{
		const FString Message = FString::Printf(TEXT("Failed to load %s library"), *Path);
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Message));
	}
#endif //WITH_EDITOR
}
#endif

void FInworldAIClientModule::StartupModule()
{
#ifdef INWORLD_WITH_NDK
	FString DllDirectory = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("InworldAI"))->GetBaseDir(), TEXT("Source/ThirdParty/InworldAINDKLibrary/lib"));

	FString LibraryPath;
#if PLATFORM_WINDOWS
	LibraryPath = FPaths::Combine(*DllDirectory, TEXT("Win64/webrtc_aec_plugin.dll"));
#endif //PLATFORM_WINDOWS
#if PLATFORM_MAC
	LibraryPath = FPaths::Combine(*DllDirectory, TEXT("Mac/libwebrtc_aec_plugin.dylib"));
#endif //PLATFORM_MAC
#ifdef INWORLD_AEC
	LoadDll(LibraryPath, &webrtcLibraryHandle);
#endif //INWORLD_AEC

#ifdef INWORLD_VAD
#if PLATFORM_WINDOWS
	LoadDll(FPaths::Combine(*DllDirectory, TEXT("Win64/inworld-ndk-vad.dll")), &vadLibHandle);
#elif PLATFORM_MAC
	LoadDll(FPaths::Combine(*DllDirectory, TEXT("Mac/libinworld-ndk-vad.dylib")), &vadLibHandle);
#endif
#endif

#ifdef INWORLD_NDK_SHARED
#if PLATFORM_WINDOWS
	LibraryPath = FPaths::Combine(*DllDirectory, TEXT("Win64/inworld-ndk.dll"));
#elif PLATFORM_MAC
	LibraryPath = FPaths::Combine(*DllDirectory, TEXT("Mac/libinworld-ndk.dylib"));
#elif PLATFORM_IOS
	LibraryPath = FPaths::Combine(*DllDirectory, TEXT("iOS/libinworld-ndk.dylib"));
#elif PLATFORM_ANDROID
	LibraryPath = FPaths::Combine(*DllDirectory, TEXT("Android/arm64-v8a/libinworld-ndk.so"));
#endif
	LoadDll(LibraryPath, &ndkLibraryHandle);
#endif


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

#ifdef INWORLD_AEC
	FPlatformProcess::FreeDllHandle(webrtcLibraryHandle);
#endif //INWORLD_AEC
	webrtcLibraryHandle = nullptr;

#ifdef INWORLD_NDK_SHARED
	FPlatformProcess::FreeDllHandle(ndkLibraryHandle);
#endif // INWORLD_NDK_SHARED
	ndkLibraryHandle = nullptr;

#ifdef INWORLD_VAD
	FPlatformProcess::FreeDllHandle(vadLibHandle);
#endif
	vadLibHandle = nullptr;
#endif
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FInworldAIClientModule, InworldAIClient)
