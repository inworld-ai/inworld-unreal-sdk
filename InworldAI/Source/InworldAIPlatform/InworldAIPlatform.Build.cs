/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

using System.IO;
using UnrealBuildTool;

public class InworldAIPlatform : ModuleRules
{
    public InworldAIPlatform(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        //bUseUnity = false;

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
