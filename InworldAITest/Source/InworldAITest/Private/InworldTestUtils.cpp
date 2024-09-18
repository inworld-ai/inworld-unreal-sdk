/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldTestUtils.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"

TArray<uint8> Inworld::Test::GetTestAudioData()
{
	const FString FilePath = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("InworldAITest"))->GetBaseDir(), TEXT("SourceAudio/TestAudio.wav"));

	TArray<uint8> Bytes;
	FFileHelper::LoadFileToArray(Bytes, *FilePath);
	return Bytes;
}