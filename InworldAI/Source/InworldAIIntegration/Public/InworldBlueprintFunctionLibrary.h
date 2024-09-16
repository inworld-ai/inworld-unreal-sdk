/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "InworldStudioTypes.h"
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
	UFUNCTION(BlueprintPure, Category = "Inworld|Component", meta = (BlueprintAutocast, DisplayName = "To Inworld Session (Inworld Session Component)", CompactNodeTitle = "->"))
	static UInworldSession* Conv_InworldSessionComponentToSession(UInworldSessionComponent* SessionComponent);

	UFUNCTION(BlueprintPure, Category = "Inworld|Component", meta = (BlueprintAutocast, DisplayName = "To Inworld Character (Inworld Character Component)", CompactNodeTitle = "->"))
	static UInworldCharacter* Conv_InworldCharacterComponentToCharacter(UInworldCharacterComponent* CharacterComponent);

	UFUNCTION(BlueprintPure, Category = "Inworld|Component", meta = (BlueprintAutocast, DisplayName = "To Inworld Player (Inworld Player Component)", CompactNodeTitle = "->"))
	static UInworldPlayer* Conv_InworldPlayerComponentToPlayer(UInworldPlayerComponent* PlayerComponent);

	UFUNCTION(BlueprintPure, Category = "Inworld|Component", meta = (DisplayName = "Get Inworld AI Plugin Version"))
	static FString GetInworldAIPluginVersion();

	UFUNCTION(BlueprintPure, Category = "Inworld|Component")
	static FString GetStudioApiKey();

	DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnInworldStudioWorkspaces, const FInworldStudioWorkspaces&, Workspaces, bool, bSuccess, const FString&, Error);
	UFUNCTION(BlueprintCallable, Category = "Inworld|Studio", meta = (AdvancedDisplay = "1", AutoCreateRefTerm = "StudioApiKeyOverride"))
	static void GetInworldStudioWorkspaces(const FOnInworldStudioWorkspaces& Callback, const FString& StudioApiKeyOverride);

	DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnInworldStudioApiKeys, const FInworldStudioApiKeys&, ApiKeys, bool, bSuccess, const FString&, Error);
	UFUNCTION(BlueprintCallable, Category = "Inworld|Studio", meta = (AdvancedDisplay = "2", AutoCreateRefTerm = "StudioApiKeyOverride"))
	static void GetInworldStudioApiKeys(const FOnInworldStudioApiKeys& Callback, const FString& Workspace, const FString& StudioApiKeyOverride);

	DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnInworldStudioCharacters, const FInworldStudioCharacters&, Characters, bool, bSuccess, const FString&, Error);
	UFUNCTION(BlueprintCallable, Category = "Inworld|Studio", meta = (AdvancedDisplay = "2", AutoCreateRefTerm = "StudioApiKeyOverride"))
	static void GetInworldStudioCharacters(const FOnInworldStudioCharacters& Callback, const FString& Workspace, const FString& StudioApiKeyOverride);

	DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnInworldStudioScenes, const FInworldStudioScenes&, Scenes, bool, bSuccess, const FString&, Error);
	UFUNCTION(BlueprintCallable, Category = "Inworld|Studio", meta = (AdvancedDisplay = "2", AutoCreateRefTerm = "StudioApiKeyOverride"))
	static void GetInworldStudioScenes(const FOnInworldStudioScenes& Callback, const FString& Workspace, const FString& StudioApiKeyOverride);

	UFUNCTION(BlueprintCallable, Category = "Inworld|Audio")
	static bool SoundWaveToDataArray(USoundWave* SoundWave, TArray<uint8>& OutDataArray);

	UFUNCTION(BlueprintCallable, Category = "Inworld|Audio")
	static USoundWave* DataArrayToSoundWave(const TArray<uint8>& DataArray);
};
