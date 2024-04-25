/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldAINDK.h"
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
	webrtcLibraryHandle = !LibraryPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*LibraryPath) : nullptr;
#if WITH_EDITOR
	if (webrtcLibraryHandle == nullptr)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("InworldAINDKModuleError", "Failed to load webrtc library"));
	}
#endif //WITH_EDITOR
#endif //INWORLD_AEC

	LibraryPath = FPaths::Combine(*DllDirectory, TEXT("Win64/inworld_onnxruntime.dll"));
	onnxLibraryHandle = !LibraryPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*LibraryPath) : nullptr;
	if (onnxLibraryHandle == nullptr)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("InworldAINDKModuleError", "Failed to load onnx library"));
	}

	LibraryPath = FPaths::Combine(*DllDirectory, TEXT("Win64/inworld_onnxruntime_providers_shared.dll"));
	onnxProvidersLibraryHandle = !LibraryPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*LibraryPath) : nullptr;
	if (onnxProvidersLibraryHandle == nullptr)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("InworldAINDKModuleError", "Failed to load onnx providers library"));
	}

	int a = 0;
	a = 2;
	
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
	ndkLibraryHandle = !LibraryPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*LibraryPath) : nullptr;
#if WITH_EDITOR
	if (ndkLibraryHandle == nullptr)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("InworldAINDKModuleError", "Failed to load ndk library"));
	}
#endif //WITH_EDITOR
#endif
}

void FInworldAINDKModule::ShutdownModule()
{
#ifdef INWORLD_AEC
	FPlatformProcess::FreeDllHandle(webrtcLibraryHandle);
#endif //INWORLD_AEC
	webrtcLibraryHandle = nullptr;

	FPlatformProcess::FreeDllHandle(onnxLibraryHandle);
	onnxLibraryHandle = nullptr;
	FPlatformProcess::FreeDllHandle(onnxProvidersLibraryHandle);
	onnxProvidersLibraryHandle = nullptr;

#ifdef INWORLD_NDK_SHARED
	FPlatformProcess::FreeDllHandle(ndkLibraryHandle);
#endif // INWORLD_NDK_SHARED
	ndkLibraryHandle = nullptr;

}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FInworldAINDKModule, InworldAINDK)
