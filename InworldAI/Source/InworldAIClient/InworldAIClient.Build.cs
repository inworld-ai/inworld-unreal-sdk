/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

using System.IO;
using System.Collections.Generic;
using UnrealBuildTool;
using System;

public class InworldAIClient : ModuleRules
{

    public InworldAIClient(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        //bUseUnity = false;

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
            });

<<<<<<< HEAD
        if (IsNDKPlatform)
=======
        if (Target.Platform == UnrealTargetPlatform.Win64 || Target.Platform == UnrealTargetPlatform.Mac || Target.Platform == UnrealTargetPlatform.IOS || Target.Platform == UnrealTargetPlatform.Android || Target.Platform == UnrealTargetPlatform.Linux)
>>>>>>> 3ec73530cd2d0f8e9b027851e8d84265d9ac88f7
        {
            PublicDefinitions.Add("INWORLD_WITH_NDK=1");
            PrivateDependencyModuleNames.AddRange(
                new string[]
                {
                    "InworldAINDKLibrary",
                });
        }
    }
    private bool IsNDKPlatform
    {
        get
        {
            return InworldAIPlatform.IsWin64(Target)
                || InworldAIPlatform.IsMac(Target)
                || InworldAIPlatform.IsIOS(Target)
                || InworldAIPlatform.IsAndroid(Target)
                || InworldAIPlatform.IsXSX(Target);
        } 
    }
}
