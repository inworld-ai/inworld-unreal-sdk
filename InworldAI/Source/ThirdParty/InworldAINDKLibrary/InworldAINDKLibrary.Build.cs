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
            if (IsWin64)
            {
                return Path.Combine(ModuleDirectory, "lib/Win64");
            }
            else if (IsMac)
            {
                return Path.Combine(ModuleDirectory, "lib/Mac");
            }
            else if (IsIOS)
            {
                return Path.Combine(ModuleDirectory, "lib/iOS");
            }
            else if (IsAndroid)
            {
                return Path.Combine(ModuleDirectory, "lib/Android/arm64-v8a");
            }
<<<<<<< HEAD
            else if (IsXSX)
            {
                return Path.Combine(ModuleDirectory, "lib/XSX");
=======
            else if (Target.Platform == UnrealTargetPlatform.Linux)
            {
            	return Path.Combine(ModuleDirectory, "lib/Linux/x86_64");
>>>>>>> 3ec73530cd2d0f8e9b027851e8d84265d9ac88f7
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
        
<<<<<<< HEAD
        bool bNDKPlatform = IsWin64 || IsMac || IsIOS || IsAndroid || IsXSX;
=======
        bool bNDKPlatform = Target.Platform == UnrealTargetPlatform.Win64 || Target.Platform == UnrealTargetPlatform.Mac || Target.Platform == UnrealTargetPlatform.IOS || Target.Platform == UnrealTargetPlatform.Android || Target.Platform == UnrealTargetPlatform.Linux;
>>>>>>> 3ec73530cd2d0f8e9b027851e8d84265d9ac88f7
        if(!bNDKPlatform)
        {
            return;
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

        // Audio Echo Cancellation (AEC) and Audio Dump supported on Windows and Mac only
        if (IsWin64 || IsMac)
        {
            PublicDefinitions.Add("INWORLD_AEC=1");
            PublicDefinitions.Add("INWORLD_AUDIO_DUMP=1");
        }
        
        // Voice Activity Detection (VAD) supported on Windows and Mac
        bool bVAD = IsWin64 || IsMac;
        if (bVAD)
        {
	        PublicDefinitions.Add("INWORLD_VAD=1");
        }

        PublicDefinitions.Add("INWORLD_LOG=1");
        PublicDefinitions.Add("INWORLD_LOG_CALLBACK=1");

<<<<<<< HEAD
        bool bUseSharedInworldNDK = !IsIOS && !IsXSX;
=======
        bool bUseSharedInworldNDK = Target.Platform != UnrealTargetPlatform.IOS && Target.Platform != UnrealTargetPlatform.Linux;
>>>>>>> 3ec73530cd2d0f8e9b027851e8d84265d9ac88f7
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
                if (IsWin64 || IsXSX)
                {
                    Name = string.Concat(Name, ".lib");
                }
<<<<<<< HEAD
                else if (IsMac ||
                    IsIOS ||
                    IsAndroid)
=======
                else if (Target.Platform == UnrealTargetPlatform.Mac ||
                    Target.Platform == UnrealTargetPlatform.IOS ||
                    Target.Platform == UnrealTargetPlatform.Android ||
                    Target.Platform == UnrealTargetPlatform.Linux)
>>>>>>> 3ec73530cd2d0f8e9b027851e8d84265d9ac88f7
                {
                    Name = Name.IndexOf("lib") != 0 ?
                        string.Concat("lib", Name, ".a") : string.Concat(Name, ".a");
                }
                PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, Name));
            }
        }

        if (IsWin64)
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
        else if(IsMac)
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
        else if(IsIOS && bUseSharedInworldNDK)
        {
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "libinworld-ndk.dylib"));
            RuntimeDependencies.Add(Path.Combine(ThirdPartyLibrariesDirectory, "libinworld-ndk.dylib"));
            PublicDelayLoadDLLs.Add(Path.Combine(ThirdPartyLibrariesDirectory, "libinworld-ndk.dylib"));
        }
        else if(IsAndroid && bUseSharedInworldNDK)
        {
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "libinworld-ndk.so"));
            RuntimeDependencies.Add(Path.Combine(ThirdPartyLibrariesDirectory, "libinworld-ndk.so"));

            string ModulePath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);
            AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(ModulePath, "InworldNDK_UPL.xml"));
        }

<<<<<<< HEAD
        if (IsWin64 || IsMac || IsAndroid || IsXSX)
=======
        if (Target.Platform == UnrealTargetPlatform.Win64 || Target.Platform == UnrealTargetPlatform.Mac || Target.Platform == UnrealTargetPlatform.Android || Target.Platform == UnrealTargetPlatform.Linux)
>>>>>>> 3ec73530cd2d0f8e9b027851e8d84265d9ac88f7
        {
            AddEngineThirdPartyPrivateStaticDependencies(Target, "zlib");
        }
        else if (IsIOS)
        {
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "libz.a"));
        }
    }
    private bool IsWin64 { get { return InworldAIPlatform.IsWin64(Target); } }
    private bool IsMac { get { return InworldAIPlatform.IsMac(Target); } }
    private bool IsIOS { get { return InworldAIPlatform.IsIOS(Target); } }
    private bool IsAndroid { get { return InworldAIPlatform.IsAndroid(Target); } }
    private bool IsLinux { get { return InworldAIPlatform.IsLinux(Target); } }
    private bool IsXSX { get { return InworldAIPlatform.IsXSX(Target); } }
}
