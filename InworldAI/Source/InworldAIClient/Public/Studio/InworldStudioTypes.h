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
struct FInworldStudioWorkspace
{
public:
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Workspace", meta = (DisplayName = "Name"))
	FString name;

	UPROPERTY(BlueprintReadOnly, Category = "Workspace", meta = (DisplayName = "Display Name"))
	FString displayName;
};

USTRUCT(BlueprintType)
struct FInworldStudioWorkspaces
{
public:
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Workspace", meta = (DisplayName = "Workspaces"))
	TArray<FInworldStudioWorkspace> workspaces;
};

USTRUCT(BlueprintType)
struct FInworldStudioApiKey
{
public:
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Api Key", meta = (DisplayName = "Key"))
	FString key;

	UPROPERTY(BlueprintReadOnly, Category = "Api Key", meta = (DisplayName = "Secret"))
	FString secret;
};

USTRUCT(BlueprintType)
struct FInworldStudioApiKeys
{
public:
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Workspace", meta = (DisplayName = "Api Keys"))
	TArray<FInworldStudioApiKey> apiKeys;
};

USTRUCT(BlueprintType)
struct FInworldStudioCharacterDescription
{
public:
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Character|Description", meta = (DisplayName = "Given Name"))
	FString givenName;
};

USTRUCT(BlueprintType)
struct FInworldStudioCharacterAssets
{
public:
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Character|Assets", meta = (DisplayName = "Avatar Image"))
	FString avatarImg;
};

USTRUCT(BlueprintType)
struct FInworldStudioCharacter
{
public:
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (DisplayName = "Name"))
	FString name;

	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (DisplayName = "Character Description"))
	FInworldStudioCharacterDescription defaultCharacterDescription;
	
	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (DisplayName = "Character Assets"))
	FInworldStudioCharacterAssets defaultCharacterAssets;
};

USTRUCT(BlueprintType)
struct FInworldStudioCharacters
{
public:
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (DisplayName = "Characters"))
	TArray<FInworldStudioCharacter> characters;
};

USTRUCT(BlueprintType)
struct FInworldStudioSceneCharacter
{
public:
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Scene|Character", meta = (DisplayName = "Name"))
	FString character;
};

USTRUCT(BlueprintType)
struct FInworldStudioScene
{
public:
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Scene", meta = (DisplayName = "Name"))
	FString name;

	UPROPERTY(BlueprintReadOnly, Category = "Scene", meta = (DisplayName = "Display Name"))
	FString displayName;

	UPROPERTY(BlueprintReadOnly, Category = "Scene", meta = (DisplayName = "Characters"))
	TArray<FInworldStudioSceneCharacter> characters;
};

USTRUCT(BlueprintType)
struct FInworldStudioScenes
{
public:
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Scene", meta = (DisplayName = "Scenes"))
	TArray<FInworldStudioScene> scenes;
};

