/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldTestChecks.h"
#include "InworldAITestModule.h"

bool Inworld::Test::CheckEqual(const TCHAR* What, const TCHAR* Actual, const TCHAR* Expected)
{
	if (FCString::Strcmp(Actual, Expected) != 0)
	{
		UE_LOG(LogInworldAITest, Error, TEXT("Expected '%s' to be \"%s\", but it was \"%s\"."), What, Expected, Actual);
		return false;
	}
	return true;
}

bool Inworld::Test::CheckEqualInsensitive(const TCHAR* What, const TCHAR* Actual, const TCHAR* Expected)
{
	if (FCString::Stricmp(Actual, Expected) != 0)
	{
		UE_LOG(LogInworldAITest, Error, TEXT("Expected '%s' to be \"%s\", but it was \"%s\"."), What, Expected, Actual);
		return false;
	}
	return true;
}

bool Inworld::Test::CheckFalse(const TCHAR* What, bool Value)
{
	if (Value)
	{
		UE_LOG(LogInworldAITest, Error, TEXT("Expected '%s' to be false."), What);
		return false;
	}
	return true;
}

bool Inworld::Test::CheckTrue(const TCHAR* What, bool Value)
{
	if (!Value)
	{
		UE_LOG(LogInworldAITest, Error, TEXT("Expected '%s' to be true."), What);
		return false;
	}
	return true;
}