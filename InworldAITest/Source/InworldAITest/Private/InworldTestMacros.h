/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#define DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND(CommandName) \
	DEFINE_LATENT_AUTOMATION_COMMAND(F##CommandName##Command); \
	void CommandName() { ADD_LATENT_AUTOMATION_COMMAND(F##CommandName##Command()); } \

#define DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(CommandName, ParamType, ParamName) \
	DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(F##CommandName##Command, ParamType, ParamName); \
	void CommandName(ParamType ParamName) { ADD_LATENT_AUTOMATION_COMMAND(F##CommandName##Command(ParamName)); } \

#define DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(CommandName, ParamType0, ParamName0, ParamType1, ParamName1) \
	DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(F##CommandName##Command, ParamType0, ParamName0, ParamType1, ParamName1); \
	void CommandName(ParamType0 ParamName0, ParamType1 ParamName1) { ADD_LATENT_AUTOMATION_COMMAND(F##CommandName##Command(ParamName0, ParamName1)); } \

#define DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(CommandName, ParamType0, ParamName0, ParamType1, ParamName1, ParamType2, ParamName2) \
	DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(F##CommandName##Command, ParamType0, ParamName0, ParamType1, ParamName1, ParamType2, ParamName2); \
	void CommandName(ParamType0 ParamName0, ParamType1 ParamName1, ParamType2 ParamName2) { ADD_LATENT_AUTOMATION_COMMAND(F##CommandName##Command(ParamName0, ParamName1, ParamName2)); } \

#define DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_FOUR_PARAMETER(CommandName, ParamType0, ParamName0, ParamType1, ParamName1, ParamType2, ParamName2, ParamType3, ParamName3) \
	DEFINE_LATENT_AUTOMATION_COMMAND_FOUR_PARAMETER(F##CommandName##Command, ParamType0, ParamName0, ParamType1, ParamName1, ParamType2, ParamName2, ParamType3, ParamName3); \
	void CommandName(ParamType0 ParamName0, ParamType1 ParamName1, ParamType2 ParamName2, ParamType3 ParamName3) { ADD_LATENT_AUTOMATION_COMMAND(F##CommandName##Command(ParamName0, ParamName1, ParamName2, ParamName3)); } \

#define DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_FIVE_PARAMETER(CommandName, ParamType0, ParamName0, ParamType1, ParamName1, ParamType2, ParamName2, ParamType3, ParamName3, ParamType4, ParamName4) \
	DEFINE_LATENT_AUTOMATION_COMMAND_FIVE_PARAMETER(F##CommandName##Command, ParamType0, ParamName0, ParamType1, ParamName1, ParamType2, ParamName2, ParamType3, ParamName3, ParamType4, ParamName4); \
	void CommandName(ParamType0 ParamName0, ParamType1 ParamName1, ParamType2 ParamName2, ParamType3 ParamName3, ParamType4 ParamName4) { ADD_LATENT_AUTOMATION_COMMAND(F##CommandName##Command(ParamName0, ParamName1, ParamName2, ParamName3, ParamName4)); } \
