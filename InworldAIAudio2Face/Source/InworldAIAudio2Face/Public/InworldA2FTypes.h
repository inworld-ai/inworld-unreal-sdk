/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once


#include "CoreMinimal.h"

#include "InworldA2FTypes.generated.h"

USTRUCT(BlueprintType)
struct INWORLDAIAUDIO2FACE_API FA2FBlendShapeData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	TMap<FName, float> Map;
};
