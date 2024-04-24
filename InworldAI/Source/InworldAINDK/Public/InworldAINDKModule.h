/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "Modules/ModuleManager.h"

class FInworldAINDKModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void* ndkLibraryHandle;
	void* webrtcLibraryHandle;

public:
	static inline FInworldAINDKModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FInworldAINDKModule>("InworldAINDK");
	}

	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("InworldAINDK");
	}
};
