/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "Interfaces/IPluginManager.h"

namespace Inworld
{
	namespace Test
	{
		bool CheckEqual(const TCHAR* What, const TCHAR* Actual, const TCHAR* Expected)
		{
			if (FCString::Strcmp(Actual, Expected) != 0)
			{
				UE_LOG(LogInworldAITest, Error, TEXT("Expected '%s' to be \"%s\", but it was \"%s\"."), What, Expected, Actual);
				return false;
			}
			return true;
		}

		bool CheckEqualInsensitive(const TCHAR* What, const TCHAR* Actual, const TCHAR* Expected)
		{
			if (FCString::Stricmp(Actual, Expected) != 0)
			{
				UE_LOG(LogInworldAITest, Error, TEXT("Expected '%s' to be \"%s\", but it was \"%s\"."), What, Expected, Actual);
				return false;
			}
			return true;
		}

		bool CheckFalse(const TCHAR* What, bool Value)
		{
			if (Value)
			{
				UE_LOG(LogInworldAITest, Error, TEXT("Expected '%s' to be false."), What);
				return false;
			}
			return true;
		}

		bool CheckTrue(const TCHAR* What, bool Value)
		{
			if (!Value)
			{
				UE_LOG(LogInworldAITest, Error, TEXT("Expected '%s' to be true."), What);
				return false;
			}
			return true;
		}
	}
}
