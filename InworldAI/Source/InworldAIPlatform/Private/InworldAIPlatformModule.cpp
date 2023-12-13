// Copyright 2023 Theai, Inc. (DBA Inworld) All Rights Reserved.

#include "InworldAIPlatformModule.h"

#if PLATFORM_IOS || PLATFORM_MAC
#include "AppleMicrophoneImpl.h"
#elif PLATFORM_ANDROID
#include "AndroidMicrophoneImpl.h"
#else
#include "GenericMicrophoneImpl.h"
#endif

#define LOCTEXT_NAMESPACE "FInworldAIPlatformModule"

void FInworldAIPlatformModule::StartupModule()
{
#if PLATFORM_IOS || PLATFORM_MAC
    PlatformMicrophone = MakeUnique<Inworld::Platform::AppleMicrophoneImpl>();
#elif PLATFORM_ANDROID
    PlatformMicrophone = MakeUnique<Inworld::Platform::AndroidMicrophoneImpl>();
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
