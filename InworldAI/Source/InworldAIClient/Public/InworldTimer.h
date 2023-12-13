// Copyright 2023 Theai, Inc. (DBA Inworld) All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UWorld;

namespace Inworld
{
    namespace Utils
    {
        class INWORLDAICLIENT_API FWorldTimer
        {
        public:
            FWorldTimer(float InThreshold)
                : Threshold(InThreshold)
            {}
             
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
