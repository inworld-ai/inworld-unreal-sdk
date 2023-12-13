// Copyright 2023 Theai, Inc. (DBA Inworld) All Rights Reserved.

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
	float* operator[](const FString& Code)
	{
		float** Viseme = TMap<FString, float*>{
			{"PP", &PP }, {"FF", &FF }, {"TH", &TH }, {"DD", &DD },{"Kk", &Kk }, {"CH", &CH }, {"SS", &SS },
			{"Nn", &Nn }, {"RR", &RR }, {"Aa", &Aa }, {"E", &E }, {"I", &I }, {"O", &O }, {"U", &U },
		}.Find(Code);
		return Viseme != nullptr ? *Viseme : &STOP;
	}
};
