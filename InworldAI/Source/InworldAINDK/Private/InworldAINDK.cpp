/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldAINDK.h"
#include "Core.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"

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

	if (webrtcLibraryHandle == nullptr)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("InworldAINDKModuleError", "Failed to load webrtc library"));
	}
#endif //INWORLD_AEC
}

void FInworldAINDKModule::ShutdownModule()
{
#ifdef INWORLD_AEC
	FPlatformProcess::FreeDllHandle(webrtcLibraryHandle);
#endif //INWORLD_AEC

	webrtcLibraryHandle = nullptr;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FInworldAINDKModule, InworldAINDK)
