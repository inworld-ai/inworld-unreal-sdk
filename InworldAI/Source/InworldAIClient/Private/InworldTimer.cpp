// Copyright 2023 Theai, Inc. (DBA Inworld) All Rights Reserved.

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
