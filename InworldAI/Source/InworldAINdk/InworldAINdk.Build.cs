// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using System.Collections.Generic;
using UnrealBuildTool;

public class InworldAINdk : ModuleRules
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
                return Path.Combine(NdkDirectory, "ThirdParty/Prebuilt/Mac/x86_64");
            }
            else if (Target.Platform == UnrealTargetPlatform.IOS)
            {
                return Path.Combine(NdkDirectory, "ThirdParty/Prebuilt/iOS/Clang-1300");
            }
            else if (Target.Platform == UnrealTargetPlatform.Android)
            {
                return Path.Combine(NdkDirectory, "ThirdParty/Prebuilt/Android/arm64-v8a");
            }
            else
            {
                return Path.Combine(NdkDirectory, "ThirdParty/Prebuilt/Unknown");
            }
        }
    }

    private static void CopyFilesRecursively(string sourcePath, string targetPath)
    {
        foreach (string dirPath in Directory.GetDirectories(sourcePath, "*", SearchOption.AllDirectories))
        {
            Directory.CreateDirectory(dirPath.Replace(sourcePath, targetPath));
        }

        foreach (string newPath in Directory.GetFiles(sourcePath, "*.*",SearchOption.AllDirectories))
        {
            File.Copy(newPath, newPath.Replace(sourcePath, targetPath), true);
        }
    }

    public InworldAINdk(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        
        // copy NDK source code
        if (!Directory.Exists(Path.Combine(ModuleDirectory, "Public/NDK")))
        {
            Directory.CreateDirectory(Path.Combine(ModuleDirectory, "Public/NDK"));
            CopyFilesRecursively(Path.Combine(NdkDirectory, "src"), Path.Combine(ModuleDirectory, "Public/NDK"));
        }

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core"
            });

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "OpenSSL",
                "libcurl",
            });

        PublicDefinitions.Add("GOOGLE_PROTOBUF_NO_RTTI");
        PublicDefinitions.Add("GPR_FORBID_UNREACHABLE_CODE");
        PublicDefinitions.Add("GRPC_ALLOW_EXCEPTIONS=0");
        PublicDefinitions.Add("GOOGLE_PROTOBUF_INTERNAL_DONATE_STEAL_INLINE=0");
        
        PublicDefinitions.Add("INWORLD_LOG=1");
        PublicDefinitions.Add("INWORLD_UNREAL=1");

        // Audio Echo Cancellation (AEC) supported on Winddows only
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            PublicDefinitions.Add("INWORLD_AEC=1");
        }

        PublicIncludePaths.Add(Path.Combine(NdkDirectory, "ThirdParty/Include"));
        PublicIncludePaths.Add(Path.Combine(NdkDirectory, "ThirdParty/grpc/include"));
        PublicIncludePaths.Add(Path.Combine(NdkDirectory, "ThirdParty/grpc/third_party"));
        PublicIncludePaths.Add(Path.Combine(NdkDirectory, "ThirdParty/grpc/third_party/abseil-cpp"));
        PublicIncludePaths.Add(Path.Combine(NdkDirectory, "ThirdParty/include/protobuf/src"));
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public/NDK/Proto"));
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public/NDK/ThirdParty"));

        List<string> NdkLibs = new List<string>();
        NdkLibs.AddRange(
            new string[]
            {
                "absl_base",
                "absl_malloc_internal",
                "absl_raw_logging_internal",
                "absl_spinlock_wait",
                "absl_throw_delegate",
                "absl_bad_optional_access",
                "absl_cord",
                "absl_str_format_internal",
                "absl_strings",
                "absl_strings_internal",
                "absl_symbolize",
                "absl_stacktrace",
                "absl_graphcycles_internal",
                "absl_synchronization",
                "absl_int128",
                "absl_status",
                "absl_statusor",
                "absl_time",
                "absl_time_zone",
                "address_sorting",
                "gpr",
                "grpc",
                "grpc++",
                "cares",
                "libprotobuf",
                "re2",
                "upb",
            });

        foreach (string NdkLib in NdkLibs)
        {
            string Name = NdkLib;
            if (Target.Platform == UnrealTargetPlatform.Win64)
            {
                Name = string.Concat(Name, ".lib");
            }
            else if (Target.Platform == UnrealTargetPlatform.Mac || 
                Target.Platform == UnrealTargetPlatform.IOS || 
                Target.Platform == UnrealTargetPlatform.Android)
            {
                Name = Name.IndexOf("lib") != 0 ?
                    string.Concat("lib", Name, ".a") : string.Concat(Name, ".a");
            }
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, Name));
        }

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "webrtc_aec_plugin.dll.lib"));
            RuntimeDependencies.Add(Path.Combine("$(BinaryOutputDir)", "webrtc_aec_plugin.dll"), Path.Combine(ThirdPartyLibrariesDirectory, "webrtc_aec_plugin.dll"));
        }

        if (Target.Platform == UnrealTargetPlatform.Win64 || Target.Platform == UnrealTargetPlatform.Mac || Target.Platform == UnrealTargetPlatform.Android)
        {
            AddEngineThirdPartyPrivateStaticDependencies(Target, "zlib");
        }
        else if (Target.Platform == UnrealTargetPlatform.IOS)
        {
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "libz.a"));
        }
    }
}
