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
	/** Viseme blend value for the PP viseme. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta = (ClampMin = 0.f, ClampMax = 1.f), Category = "Viseme")
	float PP = 0.f;

	/** Viseme blend value for the FF viseme. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta = (ClampMin = 0.f, ClampMax = 1.f), Category = "Viseme")
	float FF = 0.f;

	/** Viseme blend value for the TH viseme. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta = (ClampMin = 0.f, ClampMax = 1.f), Category = "Viseme")
	float TH = 0.f;

	/** Viseme blend value for the DD viseme. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta = (ClampMin = 0.f, ClampMax = 1.f), Category = "Viseme")
	float DD = 0.f;

	/** Viseme blend value for the Kk viseme. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta = (ClampMin = 0.f, ClampMax = 1.f), Category = "Viseme")
	float Kk = 0.f;

	/** Viseme blend value for the CH viseme. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta = (ClampMin = 0.f, ClampMax = 1.f), Category = "Viseme")
	float CH = 0.f;

	/** Viseme blend value for the SS viseme. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta = (ClampMin = 0.f, ClampMax = 1.f), Category = "Viseme")
	float SS = 0.f;

	/** Viseme blend value for the Nn viseme. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta = (ClampMin = 0.f, ClampMax = 1.f), Category = "Viseme")
	float Nn = 0.f;

	/** Viseme blend value for the RR viseme. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta = (ClampMin = 0.f, ClampMax = 1.f), Category = "Viseme")
	float RR = 0.f;

	/** Viseme blend value for the Aa viseme. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta = (ClampMin = 0.f, ClampMax = 1.f), Category = "Viseme")
	float Aa = 0.f;

	/** Viseme blend value for the E viseme. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta = (ClampMin = 0.f, ClampMax = 1.f), Category = "Viseme")
	float E = 0.f;

	/** Viseme blend value for the I viseme. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta = (ClampMin = 0.f, ClampMax = 1.f), Category = "Viseme")
	float I = 0.f;

	/** Viseme blend value for the O viseme. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta = (ClampMin = 0.f, ClampMax = 1.f), Category = "Viseme")
	float O = 0.f;

	/** Viseme blend value for the U viseme. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta = (ClampMin = 0.f, ClampMax = 1.f), Category = "Viseme")
	float U = 0.f;

	/** Viseme blend value for the STOP viseme. */
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

	/** Map of blend shape names to their corresponding values. */
	UPROPERTY(BlueprintReadOnly, Category = "A2F")
	TMap<FName, float> Map;
};
