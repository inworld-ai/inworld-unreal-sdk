/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include <vector>
#include <string>

namespace Inworld
{
	namespace Utils
	{
		std::string GetSslRootCerts();
		std::string PhonemeToViseme(const std::string& Phoneme);

		std::vector<uint8_t> HmacSha256(const std::vector<uint8_t>& Data, const std::vector<uint8_t>& Key);
		std::string ToHex(const std::vector<uint8_t>& Data);
	}
}
