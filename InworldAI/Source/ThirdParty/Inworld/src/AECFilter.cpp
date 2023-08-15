/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "AECFilter.h"
#include "webrtc/aec.h"
#include <algorithm>

Inworld::AECFilter::AECFilter()
#ifdef INWORLD_AEC
	: AecHandle(WebRtcAec3_Create(16000))
#endif // INWORLD_AEC
{}

Inworld::AECFilter::~AECFilter()
{
#ifdef INWORLD_AEC
	WebRtcAec3_Free(AecHandle);
#endif // INWORLD_AEC
}

std::vector<int16_t> Inworld::AECFilter::FilterAudio(const std::vector<int16_t>& InputData, const std::vector<int16_t>& OutputData)
{
#ifdef INWORLD_AEC
	std::vector<int16_t> FilteredAudio = InputData;
	constexpr int32_t NumSamples = 160;
	const int32_t MaxSamples = std::min(InputData.size(), OutputData.size()) / NumSamples * NumSamples;

	for (int32_t i = 0; i < MaxSamples; i += NumSamples)
	{
		WebRtcAec3_BufferFarend(AecHandle, OutputData.data() + i);
		WebRtcAec3_Process(AecHandle, InputData.data() + i, FilteredAudio.data() + i);
	}
	return FilteredAudio;
#else
	return InputData;
#endif // INWORLD_AEC
}
