/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

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
        INWORLDAICLIENT_API USoundWave* StringToSoundWave(const std::string& String);
        INWORLDAICLIENT_API bool SoundWaveToString(USoundWave* SoundWave, std::string& String);

		INWORLDAICLIENT_API void DataArray16ToVec16(const TArray<int16>& Data, std::vector<int16>& VecData);
		INWORLDAICLIENT_API void DataArray16ToString(const TArray<int16>& Data, std::string& String);
		
        INWORLDAICLIENT_API void DataArrayToString(const TArray<uint8>& Data, std::string& String);
		INWORLDAICLIENT_API void StringToDataArray(const std::string& String, TArray<uint8>& Data);
		
        INWORLDAICLIENT_API void AppendDataArrayToString(const TArray<uint8>& Data, std::string& String);
        
		INWORLDAICLIENT_API void DataArraysToString(const TArray<const TArray<uint8>*>& Data, std::string& String);
		INWORLDAICLIENT_API void StringToDataArrays(const std::string& String, TArray<TArray<uint8>*>& Data, uint32 DivSize);
        
        INWORLDAICLIENT_API void DataArrayToDataArrays(const TArray<uint8>& Data, TArray<TArray<uint8>*>& Datas, uint32 DivSize);
        
        INWORLDAICLIENT_API void StringToArrayStrings(const std::string& Data, TArray<std::string*>& Datas, uint32 DivSize);

		INWORLDAICLIENT_API bool SoundWaveToVec(USoundWave* SoundWave, std::vector<int16>& data);
		INWORLDAICLIENT_API bool SoundWaveToDataArray(USoundWave* SoundWave, TArray<int16>& Data);
		INWORLDAICLIENT_API USoundWave* VecToSoundWave(const std::vector<int16>& data);

        INWORLDAICLIENT_API TArray<uint8> HmacSha256(const TArray<uint8>& Data, const TArray<uint8>& Key);
        INWORLDAICLIENT_API std::string ToHex(const TArray<uint8>& Data);

        class INWORLDAICLIENT_API FWorldTimer
        {
        public:
            FWorldTimer(float InThreshold) : Threshold(InThreshold) {}
             
            void SetOneTime(UWorld* World, float Threshold);
            bool CheckPeriod(UWorld* World);
            bool IsExpired(UWorld* World) const;
            float GetThreshold() const { return Threshold; }

        private:
            float Threshold = 0.f;
            float LastTime = 0.f;
        };
        
    }
}