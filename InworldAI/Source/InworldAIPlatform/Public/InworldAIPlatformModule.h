/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "InworldAIPlatformInterfaces.h"

class FInworldAIPlatformModule : public IModuleInterface
{
public:

    /** IModuleInterface implementation */
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
    
    Inworld::Platform::IMicrophone* GetMicrophone() const { return PlatformMicrophone.Get(); }
    
private:
    TUniquePtr<Inworld::Platform::IMicrophone> PlatformMicrophone;
};
