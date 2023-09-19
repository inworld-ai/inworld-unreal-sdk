/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once


#include "CoreMinimal.h"

#include "InworldTypes.generated.h"

USTRUCT(BlueprintType)
struct FInworldPlayerProfile
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Player")
    FString Name = "";

    UPROPERTY(BlueprintReadWrite, Category = "Player")
    FString UniqueId = "";

    UPROPERTY(BlueprintReadWrite, Category = "Player")
    TMap<FString, FString> Fields = {};
};

USTRUCT(BlueprintType)
struct FInworldCapabilitySet
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Capability")
    bool Animations = false;

    UPROPERTY(BlueprintReadWrite, Category = "Capability")
    bool Text = true;

    UPROPERTY(BlueprintReadWrite, Category = "Capability")
    bool Audio = true;

    UPROPERTY(BlueprintReadWrite, Category = "Capability")
    bool Emotions = true;

    UPROPERTY(BlueprintReadWrite, Category = "Capability")
    bool Gestures = true;

    UPROPERTY(BlueprintReadWrite, Category = "Capability")
    bool Interruptions = true;

    UPROPERTY(BlueprintReadWrite, Category = "Capability")
    bool Triggers = true;

    UPROPERTY(BlueprintReadWrite, Category = "Capability")
    bool EmotionStreaming = true;

    UPROPERTY(BlueprintReadWrite, Category = "Capability")
    bool SilenceEvents = true;

    UPROPERTY(BlueprintReadWrite, Category = "Capability")
    bool PhonemeInfo = true;

    UPROPERTY(BlueprintReadWrite, Category = "Capability")
    bool LoadSceneInSession = true;
};

USTRUCT(BlueprintType)
struct FInworldAuth
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Capability")
	FString Base64Signature = "";

    UPROPERTY(BlueprintReadWrite, Category = "Capability")
    FString ApiKey = "";

    UPROPERTY(BlueprintReadWrite, Category = "Capability")
    FString ApiSecret = "";
};

USTRUCT(BlueprintType)
struct FInworldSessionToken
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Token")
    FString Token = "";

    UPROPERTY(BlueprintReadWrite, Category = "Token")
    int64 ExpirationTime = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Token")
    FString SessionId = "";
};

USTRUCT(BlueprintType)
struct FInworldSave
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Data")
	TArray<uint8> Data;
};

USTRUCT(BlueprintType)
struct FInworldEnvironment
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Environment")
    FString AuthUrl = "";

    UPROPERTY(BlueprintReadWrite, Category = "Environment")
    FString TargetUrl = "";
};

USTRUCT(BlueprintType)
struct FInworldAgentInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Agent")
    FString BrainName = "";

    UPROPERTY(BlueprintReadWrite, Category = "Agent")
    FString AgentId = "";

    UPROPERTY(BlueprintReadWrite, Category = "Agent")
    FString GivenName = "";
};
