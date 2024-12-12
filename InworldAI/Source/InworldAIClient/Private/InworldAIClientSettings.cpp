/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */


#include "InworldAIClientSettings.h"

UInworldAIClientSettings::UInworldAIClientSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Environment = { "api-engine.inworld.ai:443", "api.inworld.ai"};
}
