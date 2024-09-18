/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

using UnrealBuildTool;

public class InworldAITest : ModuleRules
{
	public InworldAITest(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
			}
			);
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"InworldAIClient",
				"InworldAIIntegration",
				"Projects",
            }
			);

        PrivateDefinitions.Add("INWORLD_TEST_TASK_TIMEOUT=10");
        PrivateDefinitions.Add("INWORLD_TEST_BLANK_AUDIO_DURATION=3");
    }
}
