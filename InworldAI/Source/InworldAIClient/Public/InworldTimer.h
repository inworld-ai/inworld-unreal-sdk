/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

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
