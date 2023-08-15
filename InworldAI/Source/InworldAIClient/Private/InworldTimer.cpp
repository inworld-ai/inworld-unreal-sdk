/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldTimer.h"
#include <Engine/World.h>

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
