// Copyright 2023 Theai, Inc. (DBA Inworld) All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

INWORLDAIINTEGRATION_API DECLARE_LOG_CATEGORY_EXTERN(LogInworldAIIntegration, Log, All);

class INWORLDAIINTEGRATION_API FInworldAIIntegrationModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
