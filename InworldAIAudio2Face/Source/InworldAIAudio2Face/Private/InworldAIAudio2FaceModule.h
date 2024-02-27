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
