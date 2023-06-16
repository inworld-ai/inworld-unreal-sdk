/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldUtils.h"
#include "Logging/LogMacros.h"
#include "InworldAIClientModule.h"
#include "NDK/Utils/SslCredentials.h"
#include "Audio.h"
#include "Sound/SoundWave.h"
#include "Sound/SoundGroups.h"
#include "Engine/Engine.h"
#include "Misc/FileHelper.h"
#include "Engine/World.h"

#define UI UI_ST
THIRD_PARTY_INCLUDES_START
#include "openssl/hmac.h"
THIRD_PARTY_INCLUDES_END
#undef UI

USoundWave* Inworld::Utils::StringToSoundWave(const std::string& String)
{
    uint8* Data = (uint8*)String.data();
    const uint32 Num = String.size();

    FWaveModInfo WaveInfo;
    if (!WaveInfo.ReadWaveInfo(Data, Num))
    {
        return nullptr;
    }

    USoundWave* SoundWave = NewObject<USoundWave>(USoundWave::StaticClass());
    if (!ensure(SoundWave))
    {
        return nullptr;
	}

    SoundWave->Duration = *WaveInfo.pWaveDataSize / (*WaveInfo.pChannels * (*WaveInfo.pBitsPerSample / 8.f) * *WaveInfo.pSamplesPerSec);

    SoundWave->SetSampleRate(*WaveInfo.pSamplesPerSec);
    SoundWave->NumChannels = *WaveInfo.pChannels;
    SoundWave->RawPCMDataSize = WaveInfo.SampleDataSize;
    SoundWave->SoundGroup = ESoundGroup::SOUNDGROUP_Voice;
    SoundWave->RawPCMData = (uint8*)FMemory::Malloc(WaveInfo.SampleDataSize);
    FMemory::Memcpy(SoundWave->RawPCMData, WaveInfo.SampleDataStart, WaveInfo.SampleDataSize);

    return SoundWave;
}

bool Inworld::Utils::SoundWaveToString(USoundWave* SoundWave, std::string& String)
{
    //TODO: support other formats
    constexpr int32 SampleRate = 48000;
    constexpr int32 NumChannels = 2;
    if (!ensure(
        SoundWave->GetSampleRateForCurrentPlatform() == SampleRate &&
        SoundWave->NumChannels == NumChannels
    ))
    {
        return false;
    }

    const uint8* WaveData = SoundWave->RawPCMData;
    const int32 WaveDataSize = SoundWave->RawPCMDataSize;
    constexpr int32 MinSize = 0.01f * SampleRate * NumChannels * sizeof(uint16); // 10ms
    if (!ensure(WaveData && WaveDataSize > MinSize))
    {
        return false;
    }

    String.resize(WaveDataSize);
    uint8* StrData = (uint8*)String.data();
    FMemory::Memcpy(StrData, WaveData, WaveDataSize);

    uint16* pDst = (uint16*)String.data();
    uint16* pSrc = pDst + 5;
    uint16* pEnd = (uint16*)(StrData + String.size());

    // downsample to 16000 per sec and cut the second channel
    while (true)
    {
        FMemory::Memcpy(pDst, pSrc, sizeof(uint16));
        pDst += 1;
        pSrc += 6;
        if (pSrc >= pEnd)
        {
            break;
        }
    }

    const uint32 NewSampleDataSize = (uint8*)pDst - StrData - sizeof(uint16);
    String.resize(NewSampleDataSize);

    return true;
}

INWORLDAICLIENT_API void Inworld::Utils::DataArray16ToVec16(const TArray<int16>& Data, std::vector<int16>& VecData)
{
    VecData.resize(Data.Num());
    FMemory::Memcpy((void*)VecData.data(), (void*)Data.GetData(), VecData.size() * sizeof(int16));
}

INWORLDAICLIENT_API void Inworld::Utils::DataArray16ToString(const TArray<int16>& Data, std::string& String)
{
	String.resize(Data.Num() * 2);
	FMemory::Memcpy((void*)String.data(), (void*)Data.GetData(), String.size());
}

INWORLDAICLIENT_API void Inworld::Utils::DataArrayToString(const TArray<uint8>& Data, std::string& String)
{
	String.resize(Data.Num());
	FMemory::Memcpy((void*)String.data(), (void*)Data.GetData(), String.size());
}

INWORLDAICLIENT_API void Inworld::Utils::StringToDataArray(const std::string& String, TArray<uint8>& Data)
{
	Data.SetNumUninitialized(String.size());
	FMemory::Memcpy((void*)Data.GetData(), (void*)String.data(), String.size());
}

INWORLDAICLIENT_API void Inworld::Utils::AppendDataArrayToString(const TArray<uint8>& Data, std::string& String)
{
	const uint32 Size = String.size();
	String.resize(String.size() + Data.Num());
	FMemory::Memcpy((void*)(String.data() + Size), (void*)Data.GetData(), Data.Num());
}

INWORLDAICLIENT_API void Inworld::Utils::StringToDataArrays(const std::string& String, TArray<TArray<uint8>*>& Data, uint32 DivSize)
{
	uint32 Idx = 0;
	uint32 Size = 0;
	for (uint32 Head = 0; Head < String.size(); Head += Size, Idx++)
	{
		Size = FMath::Min((uint32)String.size() - Head, DivSize);
		Data[Idx]->SetNumUninitialized(Size);
		FMemory::Memcpy((void*)Data[Idx]->GetData(), (void*)(String.data() + Head), Size);
	}
}

INWORLDAICLIENT_API void Inworld::Utils::DataArrayToDataArrays(const TArray<uint8>& Data, TArray<TArray<uint8>*>& Datas, uint32 DivSize)
{
	uint32 Idx = 0;
	uint32 Size = 0;
	for (int32 Head = 0; Head < Data.Num(); Head += Size, Idx++)
	{
		Size = FMath::Min(Data.Num() - Head, (int32)DivSize);
		Datas[Idx]->SetNumUninitialized(Size);
		FMemory::Memcpy((void*)Datas[Idx]->GetData(), (void*)(Data.GetData() + Head), Size);
	}
}

INWORLDAICLIENT_API void Inworld::Utils::StringToArrayStrings(const std::string& Data, TArray<std::string*>& Datas, uint32 DivSize)
{
	uint32 Idx = 0;
	int32 Size = 0;
	for (int32 Head = 0; Head < Data.size(); Head += Size, Idx++)
	{
		Size = FMath::Min((int32)(Data.size() - Head), (int32)DivSize);
		Datas[Idx]->resize(Size);
		FMemory::Memcpy((void*)Datas[Idx]->data(), (void*)(Data.data() + Head), Size);
	}
}

INWORLDAICLIENT_API void Inworld::Utils::DataArraysToString(const TArray<const TArray<uint8>*>& Data, std::string& String)
{
	uint32 Size = 0;
	for (auto& D : Data)
	{
		Size += D->Num();
	}

	String.resize(Size);
	
	uint32 Head = 0;
	for (auto& D : Data)
	{
		FMemory::Memcpy((void*)(String.data() + Head), (void*)D->GetData(), D->Num());
		Head += D->Num();
	}
}

INWORLDAICLIENT_API bool Inworld::Utils::SoundWaveToVec(USoundWave* SoundWave, std::vector<int16>& data)
{
	constexpr int32 SampleRate = 48000;
	constexpr int32 DownsampleRate = SampleRate / 16000;
	constexpr int32 NumChannels = 2;
	if (!ensure(
		SoundWave->GetSampleRateForCurrentPlatform() == SampleRate &&
		SoundWave->NumChannels == NumChannels
	))
	{
		return false;
	}

	const int16* WaveData = (const int16*)SoundWave->RawPCMData;
	const int32 WaveDataSize = SoundWave->RawPCMDataSize;
	constexpr int32 MinSize = 0.01f * SampleRate * NumChannels * sizeof(uint16); // 10ms
	if (!ensure(WaveData && WaveDataSize > MinSize))
	{
		return false;
	}

	const auto nSamples = WaveDataSize / (NumChannels * sizeof(uint16_t) * DownsampleRate);
	data.resize(nSamples);
	int j = 0;
	for (int i = 0; i < nSamples; i++)
	{
		data[i] = WaveData[j];
		j += NumChannels * DownsampleRate;
	}

	return true;
}

INWORLDAICLIENT_API bool Inworld::Utils::SoundWaveToDataArray(USoundWave* SoundWave, TArray<int16>& Data)
{
	constexpr int32 SampleRate = 48000;
	constexpr int32 DownsampleRate = SampleRate / 16000;
	constexpr int32 NumChannels = 2;
	if (!ensure(
		SoundWave->GetSampleRateForCurrentPlatform() == SampleRate &&
		SoundWave->NumChannels == NumChannels
	))
	{
		return false;
	}

	const int16* WaveData = (const int16*)SoundWave->RawPCMData;
	const int32 WaveDataSize = SoundWave->RawPCMDataSize;
	constexpr int32 MinSize = 0.01f * SampleRate * NumChannels * sizeof(uint16); // 10ms
	if (!ensure(WaveData && WaveDataSize > MinSize))
	{
		return false;
	}

	const auto nSamples = WaveDataSize / (NumChannels * sizeof(uint16_t) * DownsampleRate);
	Data.SetNumUninitialized(nSamples);
	int j = 0;
	for (int i = 0; i < nSamples; i++)
	{
		Data[i] = WaveData[j];
		j += NumChannels * DownsampleRate;
	}

	return true;
}

INWORLDAICLIENT_API USoundWave* Inworld::Utils::VecToSoundWave(const std::vector<int16>& data)
{
	const int16* srcPtr = data.data();
	const auto nSamples = data.size();

	USoundWave* SoundWave = NewObject<USoundWave>(USoundWave::StaticClass());
	if (!ensure(SoundWave))
	{
		return nullptr;
	}

	constexpr int32 SamplesPerSec = 16000;
	SoundWave->Duration = nSamples / SamplesPerSec;

	SoundWave->SetSampleRate(SamplesPerSec);
	SoundWave->NumChannels = 1;
	SoundWave->RawPCMDataSize = nSamples * sizeof(int16);
	SoundWave->SoundGroup = ESoundGroup::SOUNDGROUP_Voice;

	SoundWave->RawPCMData = (uint8*)FMemory::Malloc(SoundWave->RawPCMDataSize);
	FMemory::Memcpy(SoundWave->RawPCMData, srcPtr, SoundWave->RawPCMDataSize);
	return SoundWave;
}

TArray<uint8> Inworld::Utils::HmacSha256(const TArray<uint8>& Data, const TArray<uint8>& Key)
{
    TArray<uint8> Res;
    Res.SetNumZeroed(32);
	uint32 OutputLen = 0;
	HMAC(EVP_sha256(), Key.GetData(), Key.Num(), Data.GetData(), Data.Num(), Res.GetData(), &OutputLen);
    return Res;
}

std::string Inworld::Utils::ToHex(const TArray<uint8>& Data)
{
    std::string Res(Data.Num() * 2, '0');
	for (int32 i = 0; i < Data.Num(); i++)
	{
		FCStringAnsi::Sprintf((char*)(Res.data()) + (i * 2), "%02x", Data[i]);
	}

	return Res;
}

void Inworld::Utils::FWorldTimer::SetOneTime(UWorld* World, float InThreshold)
{
    Threshold = InThreshold;
    LastTime = World->GetTimeSeconds();
}

bool Inworld::Utils::FWorldTimer::CheckPeriod(UWorld* World)
{
    if (IsExpired(World))
    {
        LastTime = World->GetTimeSeconds();
        return true;
    }
    return false;
}

bool Inworld::Utils::FWorldTimer::IsExpired(UWorld* World) const
{
    return World->GetTimeSeconds() > LastTime + Threshold;
}
