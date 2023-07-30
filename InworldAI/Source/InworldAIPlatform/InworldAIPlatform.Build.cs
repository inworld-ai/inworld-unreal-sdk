// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class InworldAIPlatform : ModuleRules
{
    public InworldAIPlatform(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
            });
        
        if(Target.Platform == UnrealTargetPlatform.Mac || Target.Platform == UnrealTargetPlatform.IOS)
        {
            PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public/Apple"));
        }
        else if (Target.Platform == UnrealTargetPlatform.Android)
        {
            PublicDependencyModuleNames.Add("AndroidPermission");
            PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public/Android"));
        }
        else
        {
            PrivateDefinitions.Add("INWORLD_PLATFORM_GENERIC=1");
            PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public/Generic"));
            PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private/Generic"));
        }
    }
}
