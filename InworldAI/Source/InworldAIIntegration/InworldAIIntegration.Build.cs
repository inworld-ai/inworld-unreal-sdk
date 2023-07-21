// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class InworldAIIntegration : ModuleRules
{
    private string NdkDirectory
    {
        get
        {
            return Path.GetFullPath(Path.Combine(ModuleDirectory, "../../inworld-ndk/"));
        }
    }

    private string ThirdPartyLibrariesDirectory
    {
        get
        {
            if (Target.Platform == UnrealTargetPlatform.Win64)
            {
                return Path.Combine(NdkDirectory, "ThirdParty/Prebuilt/Win64");
            }
            else if (Target.Platform == UnrealTargetPlatform.Mac)
            {
                return Path.Combine(NdkDirectory, "ThirdParty/Prebuilt/Mac");
            }
            else if (Target.Platform == UnrealTargetPlatform.IOS)
            {
                return Path.Combine(NdkDirectory, "ThirdParty/Prebuilt/iOS");
            }
            else
            {
                return Path.Combine(NdkDirectory, "ThirdParty/Prebuilt/Unknown");
            }
        }
    }

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
                "InworldAIClient",
                "InworldAIPlatform",
                "InworldAINdk",
                "AudioCaptureCore",
            });


        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "ApplicationCore",
                "AudioMixer",
                "Projects",
                "GameplayDebugger",
            }
            );

        PrivateDefinitions.Add("INWORLD_DEBUGGER_SLOT=5");

        PublicIncludePaths.Add(Path.Combine(NdkDirectory, "ThirdParty/Include"));
        PublicIncludePaths.Add(Path.Combine(NdkDirectory, "ThirdParty/grpc/include"));

        AddEngineThirdPartyPrivateStaticDependencies(Target, "zlib");
    }
}
