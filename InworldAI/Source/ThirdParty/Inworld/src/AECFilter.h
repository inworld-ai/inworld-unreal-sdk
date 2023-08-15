/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once
#include <vector>

namespace Inworld
{
	class AECFilter
	{
	public:
		AECFilter();
		~AECFilter();

		std::vector<int16_t> FilterAudio(const std::vector<int16_t>& InputData, const std::vector<int16_t>& OutputData);

	private:
		void* AecHandle = nullptr;
	};
}

