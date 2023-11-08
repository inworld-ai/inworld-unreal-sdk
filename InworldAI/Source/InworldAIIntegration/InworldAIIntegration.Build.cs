// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class InworldAIIntegration : ModuleRules
{
    public InworldAIIntegration(ReadOnlyTargetRules Target) : base(Target)
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
                "AudioCaptureCore",
                "InworldAIClient",
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

        if (Target.Platform == UnrealTargetPlatform.Win64 || Target.Platform == UnrealTargetPlatform.Linux)
        {
            /***********************************************************
                Uncomment below to enable Pixel Streaming Microphone
               Add "PixelStreaming" as Plugin used in InworldAI.uplugin

                "Plugins": [
		            {
			            "Name": "PixelStreaming",
			            "Enabled": true,
			            "PlatformAllowList": [ "Win64", "Linux" ]
		            }
	            ],
            ************************************************************/

            //PrivateDependencyModuleNames.Add("PixelStreaming");
            //PrivateDefinitions.Add("INWORLD_PIXEL_STREAMING=1");
        }

        SetupGameplayDebuggerSupport(Target);

    }
}
