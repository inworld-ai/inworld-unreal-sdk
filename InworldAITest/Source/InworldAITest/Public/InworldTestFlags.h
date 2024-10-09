/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "Misc/AutomationTest.h"

namespace Inworld
{
	namespace Test
	{
		static const EAutomationTestFlags Flags = (
			EAutomationTestFlags::EditorContext
			| EAutomationTestFlags::CommandletContext
			| EAutomationTestFlags::ClientContext
			| EAutomationTestFlags::ProductFilter);
	}
}
