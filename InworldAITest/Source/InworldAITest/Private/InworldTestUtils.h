/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"

namespace Inworld
{
	namespace Test
	{
		TArray<uint8> GetTestAudioData()
		{
			const FString FilePath = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("InworldAITest"))->GetBaseDir(), TEXT("SourceAudio/TestAudio.wav"));

			TArray<uint8> Bytes;
			FFileHelper::LoadFileToArray(Bytes, *FilePath);
			return Bytes;
		}
	}
}
