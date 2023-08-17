/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "AudioSessionDumper.h"
#include <fstream>

#include "Log.h"
#include "SharedQueue.h"

#ifdef INWORLD_AUDIO_DUMP

constexpr uint32_t gSamplesPerSec = 16000;

struct FWavHeader 
{
	uint8_t RIFF[4] = { 'R', 'I', 'F', 'F' };
	uint32_t ChunkSize;
	uint8_t WAVE[4] = { 'W', 'A', 'V', 'E' };
	uint8_t fmt[4] = { 'f', 'm', 't', ' ' };
	uint32_t Subchunk1Size = 16;
	uint16_t AudioFormat = 1;
	uint16_t NumOfChan = 1;
	uint32_t SamplesPerSec = gSamplesPerSec;
	uint32_t bytesPerSec = gSamplesPerSec * 2;
	uint16_t blockAlign = 2;
	uint16_t bitsPerSample = 16;
	uint8_t Subchunk2ID[4] = { 'd', 'a', 't', 'a' };
	uint32_t Subchunk2Size;
};

void AudioSessionDumper::OnMessage(const std::string& Msg)
{
	std::ofstream OutStream(_FileName, std::ios::binary | std::ios_base::app);
	OutStream.write((const char*)Msg.data(), Msg.size());
	OutStream.close();
}

void AudioSessionDumper::OnSessionStart(const std::string& InFileName)
{
	_FileName = InFileName;

	std::ofstream OutStream(_FileName, std::ios::binary);
	OutStream.clear();
	OutStream.close();
	Inworld::Log("Audio dump started to %s", _FileName.c_str());

}

void AudioSessionDumper::OnSessionStop()
{
	std::ifstream InStream(_FileName, std::ios::binary | std::ios::ate);
	const int32_t WaveSize = InStream.tellg();
	std::vector<uint8_t> WaveData(WaveSize);

	InStream.seekg(0, std::ios::beg);
	InStream.read(reinterpret_cast<char*>(WaveData.data()), WaveSize);
	InStream.close();

	FWavHeader WavStruct;
	WavStruct.ChunkSize = sizeof(FWavHeader) - 8;
	WavStruct.Subchunk2Size = WaveSize;

	std::ofstream OutStream(_FileName, std::ios::binary);
	OutStream.write(reinterpret_cast<const char*>(&WavStruct), sizeof(WavStruct));
	OutStream.write(reinterpret_cast<const char*>(WaveData.data()), WaveSize);
	OutStream.close();
	Inworld::Log("audio dump saved to %s", _FileName.c_str());
}
#endif