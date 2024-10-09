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
	/**
	 * Converts an Inworld Session Component to an Inworld Session.
	 * @param SessionComponent The Inworld Session Component to convert.
	 * @return The corresponding Inworld Session.
	 */
	UFUNCTION(BlueprintPure, Category = "Inworld|Component", meta = (BlueprintAutocast, DisplayName = "To Inworld Session (Inworld Session Component)", CompactNodeTitle = "->"))
	static UInworldSession* Conv_InworldSessionComponentToSession(UInworldSessionComponent* SessionComponent);

	/**
	 * Converts an Inworld Character Component to an Inworld Character.
	 * @param CharacterComponent The Inworld Character Component to convert.
	 * @return The corresponding Inworld Character.
	 */
	UFUNCTION(BlueprintPure, Category = "Inworld|Component", meta = (BlueprintAutocast, DisplayName = "To Inworld Character (Inworld Character Component)", CompactNodeTitle = "->"))
	static UInworldCharacter* Conv_InworldCharacterComponentToCharacter(UInworldCharacterComponent* CharacterComponent);

	/**
	 * Converts an Inworld Player Component to an Inworld Player.
	 * @param PlayerComponent The Inworld Player Component to convert.
	 * @return The corresponding Inworld Player.
	 */
	UFUNCTION(BlueprintPure, Category = "Inworld|Component", meta = (BlueprintAutocast, DisplayName = "To Inworld Player (Inworld Player Component)", CompactNodeTitle = "->"))
	static UInworldPlayer* Conv_InworldPlayerComponentToPlayer(UInworldPlayerComponent* PlayerComponent);

	/**
	 * Get the version of the Inworld AI Plugin.
	 * @return The version of the Inworld AI Plugin.
	 */
	UFUNCTION(BlueprintPure, Category = "Inworld|Plugin", meta = (DisplayName = "Get Inworld AI Plugin Version"))
	static FString GetInworldAIPluginVersion();

	/**
	 * Get the Studio API URL.
	 * @return The Studio API URL.
	 */
	UFUNCTION(BlueprintPure, Category = "Inworld|Studio")
	static const FString& GetStudioApiUrl();
	/**
	 * Get the Studio API key.
	 * @return The Studio API key.
	 */
	UFUNCTION(BlueprintPure, Category = "Inworld|Studio")
	static const FString& GetStudioApiKey();

	DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnInworldStudioWorkspaces, const FInworldStudioWorkspaces&, Workspaces, bool, bSuccess, const FString&, Error);
	/**
	 * Get Inworld Studio Workspaces.
	 * @param Callback The delegate to be called upon completion.
	 * @param StudioApiKeyOverride The optional Studio API key override.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inworld|Studio", meta = (AdvancedDisplay = "1", AutoCreateRefTerm = "StudioApiKeyOverride"))
	static void GetInworldStudioWorkspaces(const FOnInworldStudioWorkspaces& Callback, const FString& StudioApiKeyOverride);

	DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnInworldStudioApiKeys, const FInworldStudioApiKeys&, ApiKeys, bool, bSuccess, const FString&, Error);
	/**
	 * Get Inworld Studio Api Keys.
	 * @param Callback The delegate to be called upon completion.
	 * @param Workspace The workspace for which to retrieve the API keys.
	 * @param StudioApiKeyOverride The optional Studio API key override.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inworld|Studio", meta = (AdvancedDisplay = "2", AutoCreateRefTerm = "StudioApiKeyOverride"))
	static void GetInworldStudioApiKeys(const FOnInworldStudioApiKeys& Callback, const FString& Workspace, const FString& StudioApiKeyOverride);

	DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnInworldStudioCharacters, const FInworldStudioCharacters&, Characters, bool, bSuccess, const FString&, Error);
	/**
	 * Get Inworld Studio Characters.
	 * @param Callback The delegate to be called upon completion.
	 * @param Workspace The workspace for which to retrieve the characters.
	 * @param StudioApiKeyOverride The optional Studio API key override.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inworld|Studio", meta = (AdvancedDisplay = "2", AutoCreateRefTerm = "StudioApiKeyOverride"))
	static void GetInworldStudioCharacters(const FOnInworldStudioCharacters& Callback, const FString& Workspace, const FString& StudioApiKeyOverride);

	DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnInworldStudioScenes, const FInworldStudioScenes&, Scenes, bool, bSuccess, const FString&, Error);
	/**
	 * Get Inworld Studio Scenes.
	 * @param Callback The delegate to be called upon completion.
	 * @param Workspace The workspace for which to retrieve the scenes.
	 * @param StudioApiKeyOverride The optional Studio API key override.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inworld|Studio", meta = (AdvancedDisplay = "2", AutoCreateRefTerm = "StudioApiKeyOverride"))
	static void GetInworldStudioScenes(const FOnInworldStudioScenes& Callback, const FString& Workspace, const FString& StudioApiKeyOverride);

	/**
	 * Convert a SoundWave to a byte array.
	 * @param SoundWave The SoundWave to convert.
	 * @param OutDataArray The output byte array.
	 * @return True if successful, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inworld|Audio")
	static bool SoundWaveToDataArray(USoundWave* SoundWave, TArray<uint8>& OutDataArray);

	/**
	 * Convert a byte array to a SoundWave.
	 * @param DataArray The byte array to convert.
	 * @return The converted SoundWave.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inworld|Audio")
	static USoundWave* DataArrayToSoundWave(const TArray<uint8>& DataArray);
};
