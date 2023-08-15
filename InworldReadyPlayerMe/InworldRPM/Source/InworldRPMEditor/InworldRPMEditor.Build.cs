// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class InworldRPMEditor : ModuleRules
{
	public InworldRPMEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"glTFRuntime",
				"glTFRuntimeEditor"
			}
			);


		PrivateDependencyModuleNames.AddRange(
			new string[]
            {
                "InworldAIEditor",
                "InworldAIClient",
                "InworldAIIntegration",
                "InworldAINdk",
				"CoreUObject",
				"Engine",
			}
			);
	}
}
