/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

namespace Inworld
{
	namespace Test
	{
		FInworldAuth GetAuth()
		{
			FInworldAuth Auth;
			if (FParse::Value(FCommandLine::Get(), TEXT("InworldTestAuth="), Auth.Base64Signature))
			{
				return Auth;
			}
			const UInworldAITestSettings* InworldAITestSettings = GetDefault<UInworldAITestSettings>();
			Auth.Base64Signature = InworldAITestSettings->RuntimeApiKey;
			return Auth;
		}
	}
}
