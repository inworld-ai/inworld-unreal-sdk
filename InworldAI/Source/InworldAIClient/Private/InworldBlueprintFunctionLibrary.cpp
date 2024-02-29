/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */


#include "InworldBlueprintFunctionLibrary.h"
#include "Networking.h"
#include "InworldPacketTranslator.h"
THIRD_PARTY_INCLUDES_START
#include "Packets.h"
THIRD_PARTY_INCLUDES_END

#include "Audio.h"
#include "Sound/SoundWave.h"

bool UInworldBlueprintFunctionLibrary::SoundWaveToDataArray(USoundWave* SoundWave, TArray<uint8>& OutDataArray)
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

    OutDataArray.SetNum(WaveDataSize);
    uint8* StrData = (uint8*)OutDataArray.GetData();
    FMemory::Memcpy(StrData, WaveData, WaveDataSize);

    uint16* pDst = (uint16*)OutDataArray.GetData();
    uint16* pSrc = pDst + 5;
    uint16* pEnd = (uint16*)(StrData + OutDataArray.Num());

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
    OutDataArray.SetNum(NewSampleDataSize);

    return true;
}

USoundWave* UInworldBlueprintFunctionLibrary::DataArrayToSoundWave(const TArray<uint8>& DataArray)
{
    uint8* Data = (uint8*)DataArray.GetData();
    const uint32 Num = DataArray.Num();

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

UInworldBlueprintFunctionLibrary::FOnTestPacket UInworldBlueprintFunctionLibrary::OnTestPacket;

void UInworldBlueprintFunctionLibrary::DoTest()
{
    const FString Path = "C:\\dev\\test_output";
    int32 Index = 0;
    TArray64<uint8> Bytes;
    FFileHelper::LoadFileToArray(Bytes, *Path);
    if (Bytes.Num() == 0) return;
    uint8* Ptr = Bytes.GetData();
    uint32 HeaderSize = 0;
    for (int i = 0; i < 4; ++i)
    {
        HeaderSize <<= 8;
        HeaderSize |= *(Ptr + Index);
        ++Index;
    }
    if(HeaderSize > 0)
    {
    Inworld::A2FOldAnimationHeaderEvent Header(Ptr + Index, HeaderSize);
    InworldPacketTranslator HeaderPacketTranslator;
    Header.Accept(HeaderPacketTranslator);
    TSharedPtr<FInworldPacket> ReceivedHeaderPacket = HeaderPacketTranslator.GetPacket();

    OnTestPacket.ExecuteIfBound(ReceivedHeaderPacket);
    }
    Index += HeaderSize;

    while (Index < Bytes.Num())
    {
        uint32 BodySize = 0;
        for (int i = 0; i < 4; ++i)
        {
            BodySize <<= 8;
            BodySize |= *(Ptr + Index);
            ++Index;
        }

        Inworld::A2FOldAnimationContentEvent Body(Ptr + Index, BodySize);
        InworldPacketTranslator BodyPacketTranslator;
        Body.Accept(BodyPacketTranslator);
        TSharedPtr<FInworldPacket> ReceivedBodyPacket = BodyPacketTranslator.GetPacket();

        Index += BodySize;

        OnTestPacket.ExecuteIfBound(ReceivedBodyPacket);
    }
}
