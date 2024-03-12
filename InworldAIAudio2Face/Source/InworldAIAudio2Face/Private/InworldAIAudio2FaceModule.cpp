/*
 * /*************************************************************************************************
 * * Copyright 2022 Theai, Inc. (DBA Inworld)
 * *
 * * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 * *************************************************************************************************/
 */

// Copyright 2023 Theai, Inc. (DBA Inworld) All Rights Reserved.

#include "InworldAIAudio2FaceModule.h"

#define LOCTEXT_NAMESPACE "FInworldAIAudio2FaceModule"

THIRD_PARTY_INCLUDES_START
// THIRD_PARTY_INCLUDES
THIRD_PARTY_INCLUDES_END

DEFINE_LOG_CATEGORY(LogInworldAIAudio2Face);

void FInworldAIAudio2FaceModule::StartupModule()
{
}

void FInworldAIAudio2FaceModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FInworldAIAudio2FaceModule, InworldAIAudio2Face)
