/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "CoreGlobals.h"

namespace Inworld
{
	namespace Test
	{
		bool CheckEqual(const TCHAR* What, const TCHAR* Actual, const TCHAR* Expected);

		bool CheckEqualInsensitive(const TCHAR* What, const TCHAR* Actual, const TCHAR* Expected);

		bool CheckFalse(const TCHAR* What, bool Value);

		bool CheckTrue(const TCHAR* What, bool Value);
	}
}
