// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class InworldMetahumanEditor : ModuleRules
{

    public InworldMetahumanEditor(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
			}
			);


		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"EditorStyle",
				"UMGEditor",
				"Blutility",
				"UMG",
                "InworldAIEditor",
                "InworldAIClient",
                "InworldAIIntegration",
			}
			);
    }
}
