/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

using System.IO;
using UnrealBuildTool;

public class InworldAIEditor : ModuleRules
{

    public InworldAIEditor(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        //bUseUnity = false;

        PublicDependencyModuleNames.AddRange(
            new string[] 
            { 
                "Core", 
                "CoreUObject",
                "Engine", 
                "InputCore",
                "InworldAIClient",
            });

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "ToolMenus",
                "Slate",
                "SlateCore",
                "UnrealEd",
                "EditorSubsystem",
                "Projects",
                "HTTP",
                "Json",
                "JsonUtilities",
                "InworldAIClient",
                "InworldAIIntegration",
                "InworldAILLM",
                "InworldAINDKLibrary",
                "EditorStyle",
                "UMGEditor",
                "Blutility",
                "UMG",
                "WorkspaceMenuStructure",
            }
            );
    }
}
