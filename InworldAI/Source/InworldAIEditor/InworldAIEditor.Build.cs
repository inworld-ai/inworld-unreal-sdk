// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class InworldAIEditor : ModuleRules
{

    public InworldAIEditor(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[] 
            { 
                "Core", 
                "CoreUObject",
                "Engine", 
                "InputCore",
            });

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "ToolMenus",
                "Slate",
                "SlateCore",
                "UnrealEd",
                "Projects",
                "HTTP",
                "Json",
                "JsonUtilities",
                "InworldAIClient",
                "InworldAIIntegration",
                "InworldAINdk",
                "EditorStyle",
                "UMGEditor",
                "Blutility",
                "UMG",
            }
            );
    }
}
