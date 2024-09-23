/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "Commands/InworldTestCommandsGarbageCollection.h"

bool Inworld::Test::FAddObjectToRootCommand::Update()
{
	Object->AddToRoot();
	return true;
}

bool Inworld::Test::FRemoveObjectFromRootCommand::Update()
{
	Object->RemoveFromRoot();
	GEngine->ForceGarbageCollection(true);
	return true;
}
