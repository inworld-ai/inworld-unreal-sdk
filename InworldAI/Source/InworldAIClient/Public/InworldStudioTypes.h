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
struct FInworldStudioTokenOptions
{
public:
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Editor Client")
	FString ServerUrl;

	UPROPERTY(EditAnywhere, Category = "Editor Client")
	FString ExchangeToken;
};

USTRUCT(BlueprintType)
struct FInworldStudioUserCharacterData
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Studio")
	FString Name;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Studio")
	FString ShortName;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Studio")
	FString RpmModelUri;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Studio")
	FString RpmImageUri;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Studio")
	FString RpmPortraitUri;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Studio")
	FString RpmPostureUri;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Studio")
	bool bMale = false;

	mutable FString RpmModelData;
};

USTRUCT(BlueprintType)
struct FInworldStudioUserSceneData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Studio")
	FString Name;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Studio")
	FString ShortName;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Studio")
	TArray<FString> Characters;
};

USTRUCT(BlueprintType)
struct FInworldStudioUserApiKeyData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Studio")
	FString Name;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Studio")
	FString Key;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Studio")
	FString Secret;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Studio")
	bool IsActive = false;
};

USTRUCT(BlueprintType)
struct FInworldStudioUserWorkspaceData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Studio")
	FString Name;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Studio")
	FString ShortName;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Studio")
	TArray<FInworldStudioUserCharacterData> Characters;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Studio")
	TArray<FInworldStudioUserSceneData> Scenes;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Studio")
	TArray<FInworldStudioUserApiKeyData> ApiKeys;
};

USTRUCT(BlueprintType)
struct FInworldStudioUserData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Studio")
	TArray<FInworldStudioUserWorkspaceData> Workspaces;
};

USTRUCT()
struct FRefreshTokenRequestData
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FString grant_type;

	UPROPERTY()
	FString refresh_token;
};

USTRUCT()
struct FRefreshTokenResponseData
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FString access_token;

	UPROPERTY()
	int32 expires_in = 0;
	
	UPROPERTY()
	FString token_type;

	UPROPERTY()
	FString refresh_token;

	UPROPERTY()
	FString id_token;
	
	UPROPERTY()
	FString user_id;

	UPROPERTY()
	FString project_id;
};
