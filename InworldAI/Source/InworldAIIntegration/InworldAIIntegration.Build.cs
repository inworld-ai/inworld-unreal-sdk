// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class InworldAIIntegration : ModuleRules
{
    public InworldAIIntegration(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "InputCore",
                "AudioCaptureCore",
                "InworldAIClient",
                "PixelStreaming",
            });


        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "ApplicationCore",
                "AudioMixer",
                "InworldAIPlatform",
                "Networking",
                "Sockets",
            }
            );


        if (Target.bBuildDeveloperTools || (Target.Configuration != UnrealTargetConfiguration.Shipping && Target.Configuration != UnrealTargetConfiguration.Test))
        {
            PublicDependencyModuleNames.Add("GameplayDebugger");
            PrivateDefinitions.Add("INWORLD_DEBUGGER_SLOT=5");
        }

    }
}
