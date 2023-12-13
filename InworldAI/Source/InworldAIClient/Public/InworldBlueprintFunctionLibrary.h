// Copyright 2023 Theai, Inc. (DBA Inworld) All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "InworldBlueprintFunctionLibrary.generated.h"

class USoundWave;

/**
 * Blueprint Function Library for Inworld.
 */
UCLASS()
class INWORLDAICLIENT_API UInworldBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable, Category = "Inworld|Audio")
	static bool SoundWaveToDataArray(USoundWave* SoundWave, TArray<uint8>& OutDataArray);

	UFUNCTION(BlueprintCallable, Category = "Inworld|Audio")
	static USoundWave* DataArrayToSoundWave(const TArray<uint8>& DataArray);
};
