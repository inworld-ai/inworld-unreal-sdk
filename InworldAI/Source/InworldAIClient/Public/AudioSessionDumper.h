/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include <string>

class FAudioSessionDumper
{
public:
	void OnSessionStart(const std::string& InFileName);
	void OnSessionStop();
	void OnMessage(const std::string& Msg);

private:
	std::string FileName;
};
