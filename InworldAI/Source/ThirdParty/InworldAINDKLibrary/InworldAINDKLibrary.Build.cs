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

        // Audio Echo Cancellation (AEC) and Audio Dump supported on Windows and Mac only
        if (Target.Platform == UnrealTargetPlatform.Win64 || Target.Platform == UnrealTargetPlatform.Mac)
        {
            PublicDefinitions.Add("INWORLD_AEC=1");
            PublicDefinitions.Add("INWORLD_AUDIO_DUMP=1");
        }
        
        // Voice Activity Detection (VAD) supported on Windows and Mac
        const bool bUseVAD = false;
        bool bVAD = bUseVAD && 
            (Target.Platform == UnrealTargetPlatform.Win64 || Target.Platform == UnrealTargetPlatform.Mac);
        
        if (bVAD)
        {
	        PublicDefinitions.Add("INWORLD_VAD=1");
	        //PublicDefinitions.Add("INWORLD_SHOW_ONSCREEN_AUDIO_SEND=1");
        }

        PublicDefinitions.Add("INWORLD_LOG=1");
        PublicDefinitions.Add("INWORLD_LOG_CALLBACK=1");

        bool bUseSharedInworldNDK = Target.Platform != UnrealTargetPlatform.IOS;
        if (bUseSharedInworldNDK)
        {
            PublicDefinitions.Add("INWORLD_NDK_SHARED=1");
        }

        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "src/Public"));

        if (!bUseSharedInworldNDK)
        {
            List<string> NdkLibs = new List<string>();
            NdkLibs.AddRange(
                new string[]
                {
                "inworld-ndk",
                "inworld-ndk-proto",
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
        }

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "webrtc_aec_plugin.lib"));
            PublicDelayLoadDLLs.Add("webrtc_aec_plugin.dll");
            RuntimeDependencies.Add(Path.Combine(ThirdPartyLibrariesDirectory, "webrtc_aec_plugin.dll"));

            if (bUseSharedInworldNDK)
            {
                PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "inworld-ndk.dll.lib"));
                PublicDelayLoadDLLs.Add("inworld-ndk.dll");
                RuntimeDependencies.Add(Path.Combine(ThirdPartyLibrariesDirectory, "inworld-ndk.dll"));
            }

            if (bVAD)
            {
	            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "inworld-ndk-vad.dll.lib"));
	            PublicDelayLoadDLLs.Add("inworld-ndk-vad.dll");
	            RuntimeDependencies.Add(Path.Combine(ThirdPartyLibrariesDirectory, "inworld-ndk-vad.dll"));

	            RuntimeDependencies.Add(Path.Combine(ModuleDirectory, "resource/silero_vad_10_27_2022.onnx"));
            }
        }
        else if(Target.Platform == UnrealTargetPlatform.Mac)
        {
            PublicDelayLoadDLLs.Add(Path.Combine(ThirdPartyLibrariesDirectory, "libwebrtc_aec_plugin.dylib"));
            RuntimeDependencies.Add(Path.Combine(ThirdPartyLibrariesDirectory, "libwebrtc_aec_plugin.dylib"));

            if (bUseSharedInworldNDK)
            {
                PublicDelayLoadDLLs.Add(Path.Combine(ThirdPartyLibrariesDirectory, "libinworld-ndk.dylib"));
                RuntimeDependencies.Add(Path.Combine(ThirdPartyLibrariesDirectory, "libinworld-ndk.dylib"));
            }

            if (bVAD)
            {
	            PublicDelayLoadDLLs.Add(Path.Combine(ThirdPartyLibrariesDirectory,"libinworld-ndk-vad.dylib"));
	            RuntimeDependencies.Add(Path.Combine(ThirdPartyLibrariesDirectory, "libinworld-ndk-vad.dylib"));

	            RuntimeDependencies.Add(Path.Combine(ModuleDirectory, "resource/silero_vad_10_27_2022.onnx"));
            }
        }
        else if(Target.Platform == UnrealTargetPlatform.IOS && bUseSharedInworldNDK)
        {
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "libinworld-ndk.dylib"));
            RuntimeDependencies.Add(Path.Combine(ThirdPartyLibrariesDirectory, "libinworld-ndk.dylib"));
            PublicDelayLoadDLLs.Add(Path.Combine(ThirdPartyLibrariesDirectory, "libinworld-ndk.dylib"));
        }
        else if(Target.Platform == UnrealTargetPlatform.Android && bUseSharedInworldNDK)
        {
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "libinworld-ndk.so"));
            RuntimeDependencies.Add(Path.Combine(ThirdPartyLibrariesDirectory, "libinworld-ndk.so"));

            string ModulePath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);
            AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(ModulePath, "InworldNDK_UPL.xml"));
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
