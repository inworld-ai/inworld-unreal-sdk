/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"

#include "InworldStudioTypes.generated.h"

USTRUCT(BlueprintType)
struct FInworldStudioWorkspaceInfo {
public:
	GENERATED_BODY()
	
	/**
	 * The name of the workspace.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Workspace", meta = (DisplayName = "Workspace"))
	FString workspace;
	/**
	 * The display name of the workspace.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Workspace", meta = (DisplayName = "Display Name"))
	FString displayName;
};

USTRUCT(BlueprintType)
struct FInworldStudioProject {
public:
	GENERATED_BODY()
	
	/**
	* The name of the collection.
	*/
	UPROPERTY(BlueprintReadOnly, Category = "Project", meta = (DisplayName = "Name"))
	FString name;
	/**
	* The workspace collection's display name.
	*/
	UPROPERTY(BlueprintReadOnly, Category = "Project", meta = (DisplayName = "Display Name"))
	FString displayName;
	/**
	* The workspaces in this collection that are visible to the user.
	*/
	UPROPERTY(BlueprintReadOnly, Category = "Project", meta = (DisplayName = "Workspaces"))
	TArray<FInworldStudioWorkspaceInfo> workspaceInfo;
};

USTRUCT(BlueprintType)
struct FInworldStudioProjects {
public:
	GENERATED_BODY()
	
	/**
	* The workspace collections visible to the user.
	*/
	UPROPERTY(BlueprintReadOnly, Category = "Project", meta = (DisplayName = "Projects"))
	TArray<FInworldStudioProject> workspaceCollections;
};

USTRUCT(BlueprintType)
struct FInworldStudioWorkspace
{
public:
	GENERATED_BODY()

	/**
	 * The name of the workspace.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Workspace", meta = (DisplayName = "Name"))
	FString name;

	/**
	 * The display name of the workspace.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Workspace", meta = (DisplayName = "Display Name"))
	FString displayName;
};

USTRUCT(BlueprintType)
struct FInworldStudioWorkspaces
{
public:
	GENERATED_BODY()

	/**
	 * The collection of workspaces.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Workspace", meta = (DisplayName = "Workspaces"))
	TArray<FInworldStudioWorkspace> workspaces;
};

USTRUCT(BlueprintType)
struct FInworldStudioApiKey
{
public:
	GENERATED_BODY()

	/**
	 * The API key.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Api Key", meta = (DisplayName = "Key"))
	FString key;

	/**
	 * The secret associated with the API key.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Api Key", meta = (DisplayName = "Secret"))
	FString secret;
};

USTRUCT(BlueprintType)
struct FInworldStudioApiKeys
{
public:
	GENERATED_BODY()

	/**
	 * The collection of API keys.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Workspace", meta = (DisplayName = "Api Keys"))
	TArray<FInworldStudioApiKey> apiKeys;
};

USTRUCT(BlueprintType)
struct FInworldStudioCharacterDescription
{
public:
	GENERATED_BODY()

	/**
	 * The given name of the character.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Character|Description", meta = (DisplayName = "Given Name"))
	FString givenName;
};

USTRUCT(BlueprintType)
struct FInworldStudioCharacterAssets
{
public:
	GENERATED_BODY()

	/**
	 * The avatar image associated with the character.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Character|Assets", meta = (DisplayName = "Avatar Image"))
	FString avatarImg;
};

USTRUCT(BlueprintType)
struct FInworldStudioCharacter
{
public:
	GENERATED_BODY()

	/**
	 * The name of the character.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (DisplayName = "Name"))
	FString name;

	/**
	 * The default character description.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (DisplayName = "Character Description"))
	FInworldStudioCharacterDescription defaultCharacterDescription;
	
	/**
	 * The default character assets.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (DisplayName = "Character Assets"))
	FInworldStudioCharacterAssets defaultCharacterAssets;
};

USTRUCT(BlueprintType)
struct FInworldStudioCharacters
{
public:
	GENERATED_BODY()

	/**
	 * The collection of characters.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (DisplayName = "Characters"))
	TArray<FInworldStudioCharacter> characters;
};

USTRUCT(BlueprintType)
struct FInworldStudioSceneCharacter
{
public:
	GENERATED_BODY()

	/**
	 * The name of the character in the scene.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Scene|Character", meta = (DisplayName = "Name"))
	FString character;

	/**
	 * The display title of the character in the scene.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Scene|Character", meta = (DisplayName = "Display Name"))
	FString displayTitle;
};

USTRUCT(BlueprintType)
struct FInworldStudioScene
{
public:
	GENERATED_BODY()

	/**
	 * The name of the scene.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Scene", meta = (DisplayName = "Name"))
	FString name;

	/**
	 * The display name of the scene.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Scene", meta = (DisplayName = "Display Name"))
	FString displayName;

	/**
	 * The characters present in the scene.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Scene", meta = (DisplayName = "Characters"))
	TArray<FInworldStudioSceneCharacter> characters;
};

USTRUCT(BlueprintType)
struct FInworldStudioScenes
{
public:
	GENERATED_BODY()

	/**
	 * The collection of scenes.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Scene", meta = (DisplayName = "Scenes"))
	TArray<FInworldStudioScene> scenes;
};
