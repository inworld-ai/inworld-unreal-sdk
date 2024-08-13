/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

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
                "AudioPlatformConfiguration",
                "InworldAIPlatform",
                "Networking",
                "Sockets",
                "Projects",
                "HTTP",
                "Json",
                "JsonUtilities"
            });

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

#if UE_5_1_OR_LATER
        SetupGameplayDebuggerSupport(Target);
#else
        if (Target.bBuildDeveloperTools || (Target.Configuration != UnrealTargetConfiguration.Shipping && Target.Configuration != UnrealTargetConfiguration.Test))
        {
            PrivateDependencyModuleNames.Add("GameplayDebugger");
            PublicDefinitions.Add("WITH_GAMEPLAY_DEBUGGER=1");
        }
        else
        {
            PublicDefinitions.Add("WITH_GAMEPLAY_DEBUGGER=0");
        }
#endif
    }
}
