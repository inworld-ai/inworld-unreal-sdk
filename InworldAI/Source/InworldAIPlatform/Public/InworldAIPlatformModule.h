// Copyright 2023 Theai, Inc. (DBA Inworld) All Rights Reserved.

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
