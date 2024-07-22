/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

using System.IO;
using System.Collections.Generic;
using UnrealBuildTool;
using System;

public class InworldAINDKLibrary : ModuleRules
{

    private string ThirdPartyLibrariesDirectory
    {
        get
        {
            if (Target.Platform == UnrealTargetPlatform.Win64)
            {
                return Path.Combine(ModuleDirectory, "lib/Win64");
            }
            else if (Target.Platform == UnrealTargetPlatform.Mac)
            {
                return Path.Combine(ModuleDirectory, "lib/Mac");
            }
            else if (Target.Platform == UnrealTargetPlatform.IOS)
            {
                return Path.Combine(ModuleDirectory, "lib/iOS");
            }
            else if (Target.Platform == UnrealTargetPlatform.Android)
            {
                return Path.Combine(ModuleDirectory, "lib/Android/arm64-v8a");
            }
            else
            {
                return Path.Combine(ModuleDirectory, "lib/Unknown");
            }
        }
    }

    public InworldAINDKLibrary(ReadOnlyTargetRules Target) : base(Target)
    {
        Type = ModuleType.External;

        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

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

        // Audio Echo Cancellation (AEC) supported on Winddows and Mac only
        if (Target.Platform == UnrealTargetPlatform.Win64 || Target.Platform == UnrealTargetPlatform.Mac)
        {
            PublicDefinitions.Add("INWORLD_AEC=1");
        }

        PublicDefinitions.Add("INWORLD_LOG=1");
        PublicDefinitions.Add("INWORLD_LOG_CALLBACK=1");
		PublicDefinitions.Add("INWORLD_AUDIO_DUMP=1");

        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "include"));
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "include/grpc"));

        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "src"));
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "src/proto"));
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "src/ThirdParty"));

        List<string> NdkLibs = new List<string>();
        NdkLibs.AddRange(
            new string[]
            {
                "InworldNdk",
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
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "webrtc_aec_plugin.lib"));

            PublicDelayLoadDLLs.Add("webrtc_aec_plugin.dll");
            RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/InworldAINDKLibrary/lib/Win64/webrtc_aec_plugin.dll");
        }
        else if(Target.Platform == UnrealTargetPlatform.Mac)
        {
            PublicDelayLoadDLLs.Add(Path.Combine(ThirdPartyLibrariesDirectory, "libwebrtc_aec_plugin.dylib"));
            RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/InworldAINDKLibrary/lib/Mac/libwebrtc_aec_plugin.dylib");
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
