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
                "NNERuntimeORTCpu",
                "NNEUtils",
                "NNEOnnxruntime",
                "ORTHelper",
            });
        
        PublicIncludePaths.Add("C:/Programs/UE_5.3/Engine/Plugins/Experimental/NNERuntimeORTCpu/Source/ThirdParty/onnxruntime/Onnxruntime/Internal/core/session");
        
        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Projects",
                "InworldAINDKLibrary",
            });
    }
}
