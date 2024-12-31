/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#define WIN64
#define MAC
#define IOS
#define ANDROID
#define LINUX
//#define XSX // Requires GDK, must request from Epic @  https://forms.unrealengine.com/s/form-console-access-request

using System.IO;
using UnrealBuildTool;

public class InworldAIPlatform : ModuleRules
{
    public InworldAIPlatform(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        //bUseUnity = false;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
            });
        
        if(Target.Platform == UnrealTargetPlatform.Mac || Target.Platform == UnrealTargetPlatform.IOS)
        {
            PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public/Apple"));
        }
        else if (Target.Platform == UnrealTargetPlatform.Android)
        {
            PublicDependencyModuleNames.Add("AndroidPermission");
            PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public/Android"));
        }
        else
        {
            PrivateDefinitions.Add("INWORLD_PLATFORM_GENERIC=1");
            PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public/Generic"));
            PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private/Generic"));
        }
    }

    public static bool IsWin64(ReadOnlyTargetRules Target)
    {
#if WIN64
        return Target.Platform == UnrealTargetPlatform.Win64;
#else
        return false;
#endif
    }

    public static bool IsMac(ReadOnlyTargetRules Target)
    {
#if MAC
        return Target.Platform == UnrealTargetPlatform.Mac;
#else
        return false;
#endif
    }

    public static bool IsIOS(ReadOnlyTargetRules Target)
    {
#if IOS
        return Target.Platform == UnrealTargetPlatform.IOS;
#else
        return false;
#endif
    }

    public static bool IsAndroid(ReadOnlyTargetRules Target)
    {
#if ANDROID
        return Target.Platform == UnrealTargetPlatform.Android;
#else
        return false;
#endif
    }

    public static bool IsLinux(ReadOnlyTargetRules Target)
    {
#if LINUX
        return Target.Platform == UnrealTargetPlatform.Linux;
#else
        return false;
#endif
    }

    public static bool IsXSX(ReadOnlyTargetRules Target)
    {
#if XSX
        return Target.Platform == UnrealTargetPlatform.XSX;
#else
        return false;
#endif
    }
}
