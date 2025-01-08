/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once


#include "CoreMinimal.h"

#include "EditorSubsystem.h"
#include "InworldAIClientSettings.h"
#include "TickableEditorObject.h"
#include "InworldStudioTypes.h"
#include "InworldEditorApi.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnCharacterStudioDataAction, const FInworldStudioCharacter&, CharacterStudioData);

DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(bool, FOnCharacterStudioDataPermission, const FInworldStudioCharacter&, CharacterStudioData);

UCLASS(BlueprintType, Config = InworldAI)
class INWORLDAIEDITOR_API UInworldEditorApiSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

public:
	void NotifyRestartRequired();

	/**
	 * Binds an action for character data.
	 * @param Name The name of the action.
	 * @param Permission The permission for the action.
	 * @param Action The action to bind.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inworld", meta = (AutoCreateRefTerm = "Name"))
	void BindActionForCharacterData(const FName& Name, FOnCharacterStudioDataPermission Permission, FOnCharacterStudioDataAction Action);

	/**
	 * Unbinds an action for character data.
	 * @param Name The name of the action to unbind.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inworld", meta = (AutoCreateRefTerm = "Name"))
	void UnbindActionForCharacterData(const FName& Name);

	/**
	 * Retrieves the actions associated with character data.
	 * @param OutKeys An array to store the keys of the character data actions.
	 */
	UFUNCTION(BlueprintPure, Category = "Inworld")
	void GetCharacterDataActions(TArray<FName>& OutKeys) const;

	/**
	 * Checks if a character data action can be executed.
	 * @param Name The name of the action.
	 * @param CharacterStudioData The character studio data.
	 * @return True if the action can be executed, false otherwise.
	 */
	UFUNCTION(BlueprintPure, Category = "Inworld", meta = (AutoCreateRefTerm = "Name"))
	bool CanExecuteCharacterDataAction(const FName& Name, const FInworldStudioCharacter& CharacterStudioData);

	/**
	 * Executes a character data action.
	 * @param Name The name of the action to execute.
	 * @param CharacterStudioData The character studio data.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inworld", meta = (AutoCreateRefTerm = "Name"))
	void ExecuteCharacterDataAction(const FName& Name, const FInworldStudioCharacter& CharacterStudioData);

	bool CanSetupAssetAsInworldPlayer(const FAssetData& AssetData, bool bLogErrors = false);
	void SetupAssetAsInworldPlayer(const FAssetData& AssetData);
	void SetupBlueprintAsInworldPlayer(UBlueprint* Blueprint);

	bool CanSetupAssetAsInworldCharacter(const FAssetData& AssetData, bool bLogErrors = false);
	void SetupAssetAsInworldCharacter(const FAssetData& AssetData);
	void SetupBlueprintAsInworldCharacter(UBlueprint* Blueprint);

	bool CanSetupAssetAsInworldMetahuman(const FAssetData& AssetData, bool bLogErrors = false);
	void SetupAssetAsInworldMetahuman(const FAssetData& AssetData);
	void SetupBlueprintAsInworldMetahuman(UBlueprint* Blueprint);

	/** Subsystem interface */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UBlueprint* CreateCharacterActorBP(const FInworldStudioCharacter& CharacterData);
	void SavePackageToCharacterFolder(UObject* Object, const FInworldStudioCharacter& CharacterData, const FString& NamePrefix, FString NameSuffix = "");

	UObject* AddNodeToBlueprint(UBlueprint* Blueprint, UClass* Class, const FString& NodeName);
	UObject* AddNodeToBlueprintNode(UBlueprint* Blueprint, const FString& ParentNodeName, UClass* Class, const FString& NodeName);
	UObject* GetNodeFromBlueprint(UBlueprint* Blueprint, const FString& NodeName);

	/**
	 * Checks if an Innequin actor can be created for the given character data.
	 * @param CharacterData The character data for which the Innequin actor creation is checked.
	 * @return True if an Innequin actor can be created, false otherwise.
	 */
	UFUNCTION()
	bool CanCreateInnequinActor(const FInworldStudioCharacter& CharacterData);

	/**
	 * Creates an Innequin actor for the given character data.
	 * @param CharacterData The character data for which the Innequin actor is created.
	 */
	UFUNCTION()
	void CreateInnequinActor(const FInworldStudioCharacter& CharacterData);

private:

	struct FCharacterStudioDataFunctions
	{
	public:
		FOnCharacterStudioDataPermission Permission;
		FOnCharacterStudioDataAction Action;
	};

	TMap<FName, FCharacterStudioDataFunctions> CharacterStudioDataFunctionMap;

	TSharedPtr<class FInworldEditorRestartRequiredNotification> RestartRequiredNotification;

public:
	/**
	 * Set the data for the Studio API widget.
	 * @param StudioApiKey The API key for the Studio.
	 * @param Workspace The workspace for the Studio.
	 * @param RuntimeApiKey The runtime API key for the Studio.
	 * @param Scene The scene for the Studio.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inworld|Studio API")
	void SetStudioApiWidgetData(const FString& StudioApiKey, const FString& Project, const FString& Workspace, const FString& RuntimeApiKey, const FString& RuntimeApiSecret, const FString& Scene)
	{
		CacheStudioWidgetStudioApiKey = StudioApiKey;
		CacheStudioWidgetProject = Project;
		CacheStudioWidgetWorkspace = Workspace;
		CacheStudioWidgetRuntimeApiKey = RuntimeApiKey;
		CacheStudioWidgetScene = Scene;
gitg
		UInworldAIClientSettings* InworldAIClientSettings = GetMutableDefault<UInworldAIClientSettings>();

		FString Prefix = "workspaces/";
		FString WorkspaceID = Workspace;
		if(!WorkspaceID.IsEmpty() && WorkspaceID.StartsWith(Prefix))
		{
			WorkspaceID = WorkspaceID.RightChop(Prefix.Len());
		}
		
		InworldAIClientSettings->Workspace = WorkspaceID;
		InworldAIClientSettings->Auth.ApiKey = RuntimeApiKey;
		InworldAIClientSettings->Auth.ApiSecret = RuntimeApiSecret;
		InworldAIClientSettings->SaveConfig();
		
		SaveConfig();
	}

	/**
	 * Get the data for the Studio API widget.
	 * @param StudioApiKey The API key for the Studio.
	 * @param Workspace The workspace for the Studio.
	 * @param RuntimeApiKey The runtime API key for the Studio.
	 * @param Scene The scene for the Studio.
	 */
	UFUNCTION(BlueprintPure, Category = "Inworld|Studio Api")
	void GetStudioApiWidgetData(FString& StudioApiKey, FString& Project, FString& Workspace, FString& RuntimeApiKey, FString& Scene)
	{
		StudioApiKey = CacheStudioWidgetStudioApiKey;
		Project = CacheStudioWidgetProject;
		Workspace = CacheStudioWidgetWorkspace;
		RuntimeApiKey = CacheStudioWidgetRuntimeApiKey;
		Scene = CacheStudioWidgetScene;
	}

private:
	UPROPERTY(config)
	FString CacheStudioWidgetStudioApiKey;
	
    UPROPERTY(config)
    FString CacheStudioWidgetProject;

	UPROPERTY(config)
	FString CacheStudioWidgetWorkspace;

	UPROPERTY(config)
	FString CacheStudioWidgetRuntimeApiKey;

	UPROPERTY(config)
	FString CacheStudioWidgetScene;

public:
	/**
	 * Set the character data for the Dialogue Map.
	 * @param BrainName The name of the character's brain.
	 * @param DisplayName The display name of the character.
	 * @param ImageURI The URI for the character's image.
	 * @param ApiKey The API key for the character.
	 * @param ApiSecret The API secret for the character.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inworld|Dialogue Map")
	void SetDialogueMapCharacterData(const FString& BrainName, const FString& DisplayName, const FString& ImageURI, const FString& ApiKey, const FString& ApiSecret)
	{
		DialogueMapCharacterBrainName = BrainName;
		DialogueMapCharacterDisplayName = DisplayName;
		DialogueMapCharacterImageURI = ImageURI;
		DialogueMapCharacterApiKey = ApiKey;
		DialogueMapCharacterApiSecret = ApiSecret;
		SaveConfig();
	}

	/**
	 * Get the character data for the Dialogue Map.
	 * @param BrainName The name of the character's brain.
	 * @param DisplayName The display name of the character.
	 * @param ImageURI The URI for the character's image.
	 * @param ApiKey The API key for the character.
	 * @param ApiSecret The API secret for the character.
	 */
	UFUNCTION(BlueprintPure, Category = "Inworld|Dialogue Map")
	void GetDialogueMapCharacterData(FString& BrainName, FString& DisplayName, FString& ImageURI, FString& ApiKey, FString& ApiSecret)
	{
		BrainName = DialogueMapCharacterBrainName;
		DisplayName = DialogueMapCharacterDisplayName;
		ImageURI = DialogueMapCharacterImageURI;
		ApiKey = DialogueMapCharacterApiKey;
		ApiSecret = DialogueMapCharacterApiSecret;
	}

private:
	UPROPERTY(config)
	FString DialogueMapCharacterBrainName;

	UPROPERTY(config)
	FString DialogueMapCharacterDisplayName;

	UPROPERTY(config)
	FString DialogueMapCharacterImageURI;

	UPROPERTY(config)
	FString DialogueMapCharacterApiKey;

	UPROPERTY(config)
	FString DialogueMapCharacterApiSecret;
};
