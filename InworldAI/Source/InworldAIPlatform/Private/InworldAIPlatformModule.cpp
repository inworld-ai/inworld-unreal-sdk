/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldAIPlatformModule.h"

#if PLATFORM_IOS || PLATFORM_MAC
#include "AppleMicrophoneImpl.h"
#else
#include "GenericMicrophoneImpl.h"
#endif

#define LOCTEXT_NAMESPACE "FInworldAIPlatformModule"

void FInworldAIPlatformModule::StartupModule()
{
#if PLATFORM_IOS || PLATFORM_MAC
    PlatformMicrophone = MakeUnique<Inworld::Platform::AppleMicrophoneImpl>();
#else
    PlatformMicrophone = MakeUnique<Inworld::Platform::GenericMicrophoneImpl>();
#endif
}

void FInworldAIPlatformModule::ShutdownModule()
{
    PlatformMicrophone.Reset();
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FInworldAIPlatformModule, InworldAIPlatform)
