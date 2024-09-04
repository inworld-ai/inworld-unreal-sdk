/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

using System.IO;
using UnrealBuildTool;

public class InworldAILLM : ModuleRules
{
    public InworldAILLM(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        //bUseUnity = false;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine"
            });


        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "HTTP",
                "Json",
                "JsonUtilities"
            });
    }
}
