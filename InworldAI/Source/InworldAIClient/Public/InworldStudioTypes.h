/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "StudioClient.h"

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

FInworldStudioUserData ConvertStudioUserData(const Inworld::StudioUserData& Data)
{
	FInworldStudioUserData D;
	for (const auto& W : Data.Workspaces)
	{
		auto& WNew =  D.Workspaces.Emplace_GetRef();
		WNew.Name = UTF8_TO_TCHAR(W.Name.c_str());
		WNew.ShortName = UTF8_TO_TCHAR(W.ShortName.c_str());

		WNew.ApiKeys.Reserve(W.ApiKeys.size());
		for (const auto& A : W.ApiKeys)
		{
			auto& ANew = WNew.ApiKeys.Emplace_GetRef();
			ANew.Name = UTF8_TO_TCHAR(A.Name.c_str());
			ANew.Key = UTF8_TO_TCHAR(A.Key.c_str());
			ANew.Secret = UTF8_TO_TCHAR(A.Secret.c_str());
			ANew.IsActive = A.IsActive;
		}

		WNew.Characters.Reserve(W.Characters.size());
		for (const auto& C : W.Characters)
		{
			auto& CNew = WNew.Characters.Emplace_GetRef();
			CNew.Name = UTF8_TO_TCHAR(C.Name.c_str());
			CNew.RpmImageUri = UTF8_TO_TCHAR(C.RpmImageUri.c_str());
			CNew.RpmModelData = UTF8_TO_TCHAR(C.RpmModelData.c_str());
			CNew.RpmModelUri = UTF8_TO_TCHAR(C.RpmModelUri.c_str());
			CNew.RpmPortraitUri = UTF8_TO_TCHAR(C.RpmPortraitUri.c_str());
			CNew.RpmPostureUri = UTF8_TO_TCHAR(C.RpmPostureUri.c_str());
			CNew.bMale = C.bMale;
		}

		WNew.Scenes.Reserve(W.Scenes.size());
		for (const auto& S : W.Scenes)
		{
			auto& SNew = WNew.Scenes.Emplace_GetRef();
			SNew.Name = UTF8_TO_TCHAR(S.Name.c_str());
			SNew.ShortName = UTF8_TO_TCHAR(S.ShortName.c_str());

			SNew.Characters.Reserve(S.Characters.size());
			for (const auto& C : S.Characters)
			{
				SNew.Characters.Add(UTF8_TO_TCHAR(C.c_str()));
			}
		}
	}
	return D;
}
