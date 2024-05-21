/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldAINDKModule.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"
#include "HAL/PlatformProcess.h"
#include "Misc/Paths.h"

#if WITH_EDITOR
#include "Misc/MessageDialog.h"
#endif //WITH_EDITOR

#define LOCTEXT_NAMESPACE "FInworldAINDKModule"

void FInworldAINDKModule::StartupModule()
{
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

	//LoadDll(FPaths::Combine(*DllDirectory, TEXT("Win64/inworld-ndk-vad.dll")), &vadLibHandle);
	//LoadDll(FPaths::Combine(*DllDirectory, TEXT("Win64/onnxruntime.dll")), &onnxruntimeLibHandle);
	//LoadDll(FPaths::Combine(*DllDirectory, TEXT("Win64/onnxruntime_providers_shared.dll")), &onnxruntimeProvLibHandle);
}

void FInworldAINDKModule::ShutdownModule()
{
#ifdef INWORLD_AEC
	FPlatformProcess::FreeDllHandle(webrtcLibraryHandle);
#endif //INWORLD_AEC
	webrtcLibraryHandle = nullptr;

#ifdef INWORLD_NDK_SHARED
	FPlatformProcess::FreeDllHandle(ndkLibraryHandle);
#endif // INWORLD_NDK_SHARED
	ndkLibraryHandle = nullptr;

	FPlatformProcess::FreeDllHandle(vadLibHandle);
	vadLibHandle = nullptr;
	
	FPlatformProcess::FreeDllHandle(onnxruntimeLibHandle);
	onnxruntimeLibHandle = nullptr;
	
	FPlatformProcess::FreeDllHandle(onnxruntimeProvLibHandle);\
	onnxruntimeProvLibHandle = nullptr;
}

void FInworldAINDKModule::LoadDll(const FString& Path, void** Handle)
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

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FInworldAINDKModule, InworldAINDK)
