/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once


#include "CoreMinimal.h"

#include "InworldIntegrationTypes.generated.h"

USTRUCT(BlueprintType)
struct INWORLDAIINTEGRATION_API FInworldCharacterVisemeBlends
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta = (ClampMin = 0.f, ClampMax = 1.f), Category = "Viseme")
	float PP = 0.f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta = (ClampMin = 0.f, ClampMax = 1.f), Category = "Viseme")
	float FF = 0.f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta = (ClampMin = 0.f, ClampMax = 1.f), Category = "Viseme")
	float TH = 0.f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta = (ClampMin = 0.f, ClampMax = 1.f), Category = "Viseme")
	float DD = 0.f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta = (ClampMin = 0.f, ClampMax = 1.f), Category = "Viseme")
	float Kk = 0.f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta = (ClampMin = 0.f, ClampMax = 1.f), Category = "Viseme")
	float CH = 0.f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta = (ClampMin = 0.f, ClampMax = 1.f), Category = "Viseme")
	float SS = 0.f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta = (ClampMin = 0.f, ClampMax = 1.f), Category = "Viseme")
	float Nn = 0.f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta = (ClampMin = 0.f, ClampMax = 1.f), Category = "Viseme")
	float RR = 0.f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta = (ClampMin = 0.f, ClampMax = 1.f), Category = "Viseme")
	float Aa = 0.f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta = (ClampMin = 0.f, ClampMax = 1.f), Category = "Viseme")
	float E = 0.f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta = (ClampMin = 0.f, ClampMax = 1.f), Category = "Viseme")
	float I = 0.f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta = (ClampMin = 0.f, ClampMax = 1.f), Category = "Viseme")
	float O = 0.f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta = (ClampMin = 0.f, ClampMax = 1.f), Category = "Viseme")
	float U = 0.f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta = (ClampMin = 0.f, ClampMax = 1.f), Category = "Viseme")
	float STOP = 1.f;

public:
	float& operator[](const FString& Code);
	float& operator[](const FName& Code);
};

USTRUCT(BlueprintType)
struct INWORLDAIINTEGRATION_API FA2FBlendShapeData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	TMap<FName, float> Map;
};
