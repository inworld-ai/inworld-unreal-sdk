// Copyright 2023 Theai, Inc. (DBA Inworld) All Rights Reserved.

#pragma once


#include "CoreMinimal.h"

#include <string>
#include <vector>

class USoundWave;
class UWorld;

namespace Inworld
{
    namespace Utils
    {
        USoundWave* StringToSoundWave(const std::string& String);
        bool SoundWaveToString(USoundWave* SoundWave, std::string& String);

        void DataArray16ToVec16(const TArray<int16>& Data, std::vector<int16>& VecData);
        void DataArray16ToString(const TArray<int16>& Data, std::string& String);
		
        void DataArrayToString(const TArray<uint8>& Data, std::string& String);
        void StringToDataArray(const std::string& String, TArray<uint8>& Data);
		
        void AppendDataArrayToString(const TArray<uint8>& Data, std::string& String);
        
        void DataArraysToString(const TArray<const TArray<uint8>*>& Data, std::string& String);
        void StringToDataArrays(const std::string& String, TArray<TArray<uint8>*>& Data, uint32 DivSize);
        
        void DataArrayToDataArrays(const TArray<uint8>& Data, TArray<TArray<uint8>*>& Datas, uint32 DivSize);
        
        void StringToArrayStrings(const std::string& Data, TArray<std::string*>& Datas, uint32 DivSize);

        bool SoundWaveToVec(USoundWave* SoundWave, std::vector<int16>& data);
        bool SoundWaveToDataArray(USoundWave* SoundWave, TArray<int16>& Data);
        USoundWave* VecToSoundWave(const std::vector<int16>& data);

        TArray<uint8> HmacSha256(const TArray<uint8>& Data, const TArray<uint8>& Key);
        std::string ToHex(const TArray<uint8>& Data);
    }
}
