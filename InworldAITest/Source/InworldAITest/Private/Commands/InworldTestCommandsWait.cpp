/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "Commands/InworldTestCommandsWait.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"

void Inworld::Test::Wait(float Duration)
{
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(Duration));
}
