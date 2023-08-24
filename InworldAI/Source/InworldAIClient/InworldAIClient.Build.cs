// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using System.Collections.Generic;
using UnrealBuildTool;
using System;

public class InworldAIClient : ModuleRules
{

    public InworldAIClient(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
   
        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
            });

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Projects",
                "InworldAINdk",
            });
    }
}
