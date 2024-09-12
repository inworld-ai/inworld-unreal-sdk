/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once


#include "CoreMinimal.h"

#include "EditorSubsystem.h"
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

	UFUNCTION(BlueprintCallable, Category = "Inworld", meta = (AutoCreateRefTerm = "Name"))
	void BindActionForCharacterData(const FName& Name, FOnCharacterStudioDataPermission Permission, FOnCharacterStudioDataAction Action);

	UFUNCTION(BlueprintCallable, Category = "Inworld", meta = (AutoCreateRefTerm = "Name"))
	void UnbindActionForCharacterData(const FName& Name);

	UFUNCTION(BlueprintPure, Category = "Inworld")
	void GetCharacterDataActions(TArray<FName>& OutKeys) const;

	UFUNCTION(BlueprintPure, Category = "Inworld", meta = (AutoCreateRefTerm = "Name"))
	bool CanExecuteCharacterDataAction(const FName& Name, const FInworldStudioCharacter& CharacterStudioData);
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

	UFUNCTION()
	bool CanCreateInnequinActor(const FInworldStudioCharacter& CharacterData);
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
	UFUNCTION(BlueprintCallable, Category = "Inworld|Studio API")
	void SetStudioApiWidgetData(const FString& StudioApiKey, const FString& Workspace, const FString& RuntimeApiKey, const FString& Scene)
	{
		CacheStudioWidgetStudioApiKey = StudioApiKey;
		CacheStudioWidgetWorkspace = Workspace;
		CacheStudioWidgetRuntimeApiKey = RuntimeApiKey;
		CacheStudioWidgetScene = Scene;
		SaveConfig();
	}

	UFUNCTION(BlueprintPure, Category = "Inworld|Studio Api")
	void GetStudioApiWidgetData(FString& StudioApiKey, FString& Workspace, FString& RuntimeApiKey, FString& Scene)
	{
		StudioApiKey = CacheStudioWidgetStudioApiKey;
		Workspace = CacheStudioWidgetWorkspace;
		RuntimeApiKey = CacheStudioWidgetRuntimeApiKey;
		Scene = CacheStudioWidgetScene;
	}

private:
	UPROPERTY(config)
	FString CacheStudioWidgetStudioApiKey;

	UPROPERTY(config)
	FString CacheStudioWidgetWorkspace;

	UPROPERTY(config)
	FString CacheStudioWidgetRuntimeApiKey;

	UPROPERTY(config)
	FString CacheStudioWidgetScene;

public:
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
