/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "InworldBlueprintFunctionLibrary.generated.h"

class USoundWave;

/**
 * Blueprint Function Library for Inworld.
 */
UCLASS()
class INWORLDAIINTEGRATION_API UInworldBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintPure, meta = (BlueprintAutocast, DisplayName = "To Inworld Session (Inworld Session Component)", CompactNodeTitle = "->"))
	static UInworldSession* Conv_InworldSessionComponentToSession(UInworldSessionComponent* SessionComponent);

	UFUNCTION(BlueprintPure, meta = (BlueprintAutocast, DisplayName = "To Inworld Character (Inworld Character Component)", CompactNodeTitle = "->"))
	static UInworldCharacter* Conv_InworldCharacterComponentToCharacter(UInworldCharacterComponent* CharacterComponent);

	UFUNCTION(BlueprintPure, meta = (BlueprintAutocast, DisplayName = "To Inworld Player (Inworld Player Component)", CompactNodeTitle = "->"))
	static UInworldPlayer* Conv_InworldPlayerComponentToPlayer(UInworldPlayerComponent* PlayerComponent);

	UFUNCTION(BlueprintCallable, Category = "Inworld|Audio")
	static bool SoundWaveToDataArray(USoundWave* SoundWave, TArray<uint8>& OutDataArray);

	UFUNCTION(BlueprintCallable, Category = "Inworld|Audio")
	static USoundWave* DataArrayToSoundWave(const TArray<uint8>& DataArray);
};
