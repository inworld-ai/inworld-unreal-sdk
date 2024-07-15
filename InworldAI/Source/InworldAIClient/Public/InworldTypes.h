/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once


#include "CoreMinimal.h"
#include "InworldEnums.h"

#include "InworldTypes.generated.h"

USTRUCT(BlueprintType)
struct FInworldPlayerProfile
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Player")
    FString Name = "";

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Player")
    FString UniqueId = "";

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Environment")
    FString ProjectName = "";

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Player")
    TMap<FString, FString> Fields = {};
};

USTRUCT(BlueprintType)
struct FInworldCapabilitySet
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Capability")
    bool Animations = false;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Capability")
    bool Audio = true;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Capability")
    bool Emotions = true;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Capability")
    bool Interruptions = true;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Capability")
    bool EmotionStreaming = true;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Capability")
    bool SilenceEvents = true;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Capability")
    bool PhonemeInfo = true;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Capability")
    bool Continuation = true;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Capability")
    bool TurnBasedSTT = true;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Capability")
    bool NarratedActions = false;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Capability")
    bool Relations = true;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Capability")
    bool MultiAgent = true;
};

USTRUCT(BlueprintType)
struct FInworldAuth
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Capability")
	FString Base64Signature = "";

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Capability")
    FString ApiKey = "";

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Capability")
    FString ApiSecret = "";
};

USTRUCT(BlueprintType)
struct FInworldSessionToken
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Token")
    FString Token = "";

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Token")
    int64 ExpirationTime = 0;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Token")
    FString SessionId = "";
};

USTRUCT(BlueprintType)
struct FInworldSave
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Data")
	TArray<uint8> Data;
};

USTRUCT(BlueprintType)
struct FInworldEnvironment
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Environment")
    FString AuthUrl = "";

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Environment")
    FString TargetUrl = "";
};

USTRUCT(BlueprintType)
struct FInworldAgentInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Agent")
    FString BrainName = "";

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Agent")
    FString AgentId = "";

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Agent")
    FString GivenName = "";
};

USTRUCT(BlueprintType)
struct FInworldConnectionErrorDetails
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Agent")
    EInworldConnectionErrorType ConnectionErrorType;

    UPROPERTY(BlueprintReadOnly, Category = "Agent")
    EInworldReconnectionType ReconnectionType;

    UPROPERTY(BlueprintReadOnly, Category = "Agent")
    int64 ReconnectTime = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Agent")
    int32 MaxRetries = 0;
};
