/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */


#include "InworldAILLMSettings.h"

UInworldAILLMSettings::UInworldAILLMSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ApiUrl = { "api.inworld.ai" };
	Model = { "inworld-dragon" };
	ServiceProvider = { "SERVICE_PROVIDER_INWORLD" };
}
