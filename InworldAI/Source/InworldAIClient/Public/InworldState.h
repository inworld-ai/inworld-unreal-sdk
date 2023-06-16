/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once


#include "CoreMinimal.h"

#include "InworldState.generated.h"

UENUM(BlueprintType)
enum class EInworldConnectionState : uint8 {
    Idle,
    Connecting,
    Connected,
    Failed,
    Paused,
    Disconnected,
    Reconnecting
};