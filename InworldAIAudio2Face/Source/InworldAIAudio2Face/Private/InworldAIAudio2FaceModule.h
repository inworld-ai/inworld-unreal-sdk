/*
 * /*************************************************************************************************
 * * Copyright 2022 Theai, Inc. (DBA Inworld)
 * *
 * * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 * *************************************************************************************************/
 */

// Copyright 2023 Theai, Inc. (DBA Inworld) All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

INWORLDAIAUDIO2FACE_API DECLARE_LOG_CATEGORY_EXTERN(LogInworldAIAudio2Face, Log, All);

class FInworldAIAudio2FaceModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
