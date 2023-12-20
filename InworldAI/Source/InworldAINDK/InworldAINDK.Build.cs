// Fill out your copyright notice in the Description page of Project Settings.

using System.IO;
using UnrealBuildTool;

public class InworldAINDK : ModuleRules
{
	public InworldAINDK(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        //bUseUnity = false;

        PublicDependencyModuleNames.AddRange(
        new string[]
        {
            "Core",
            "InworldAINDKLibrary",
            "Projects"
        }
        );
    }
}
