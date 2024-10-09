/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldAITestSettings.h"

UInworldAITestSettings::UInworldAITestSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Workspace = TEXT("sdk_test_automation");

	Scene.Type = EInworldSceneType::SCENE;
	Scene.Name = TEXT("test_scene");

	InitialCharacterNames = { TEXT("character_one"), TEXT("character_two"), TEXT("character_three"), };
	CharacterNamesToLoad = { TEXT("character_four"), TEXT("character_five"), };
	CharacterNamesToUnload = { TEXT("character_two"), TEXT("character_three"), };
}
