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

    /**
     * The name of the player.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Player")
    FString Name = "";

    /**
     * The unique identifier of the player.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Player")
    FString UniqueId = "";

    /**
     * The name of the project the player is associated with.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Environment")
    FString ProjectName = "";

    /**
     * Additional fields associated with the player, stored as a key-value map.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Player")
    TMap<FString, FString> Fields = {};
};

USTRUCT(BlueprintType)
struct FInworldCapabilitySet
{
    GENERATED_BODY()

    /**
     * Indicates if the capability for handling animations is enabled.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Capability")
    bool Animations = false;

    /**
     * Indicates if the capability for handling audio is enabled.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Capability")
    bool Audio = true;

    /**
     * Indicates if the capability for handling emotions is enabled.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Capability")
    bool Emotions = true;

    /**
     * Indicates if the capability for handling interruptions is enabled.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Capability")
    bool Interruptions = true;

    /**
     * Indicates if the capability for streaming emotions is enabled.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Capability")
    bool EmotionStreaming = true;

    /**
     * Indicates if the capability for handling silence events is enabled.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Capability")
    bool SilenceEvents = true;

    /**
     * Indicates if the capability for handling phoneme information is enabled.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Capability")
    bool PhonemeInfo = true;

    /**
     * Indicates if the capability for continuation is enabled.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Capability")
    bool Continuation = true;

    /**
     * Indicates if the capability for turn-based speech-to-text is enabled.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Capability")
    bool TurnBasedSTT = true;

    /**
     * Indicates if the capability for narrated actions is enabled.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Capability")
    bool NarratedActions = false;

    /**
     * Indicates if the capability for handling relations is enabled.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Capability")
    bool Relations = true;

    /**
     * Indicates if the capability for multi-agent interactions is enabled.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Capability")
    bool MultiAgent = true;

    /**
     * Indicates if the capability for audio-driven facial animations is enabled.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Capability")
    bool Audio2Face = false;

    /**
     * Indicates if the capability for multi-modal action planning is enabled.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Capability")
    bool MultiModalActionPlanning = false;
};

USTRUCT(BlueprintType)
struct FInworldAuth
{
    GENERATED_BODY()

    /**
     * Base64 signature for authentication.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Capability")
    FString Base64Signature = "";

    /**
     * API key for authentication.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Capability")
    FString ApiKey = "";

    /**
     * API secret for authentication.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Capability")
    FString ApiSecret = "";
};

USTRUCT(BlueprintType)
struct FInworldSessionToken
{
    GENERATED_BODY()

    /**
     * The session token for authentication.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Token")
    FString Token = "";

    /**
     * The expiration time of the session token.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Token")
    int64 ExpirationTime = 0;

    /**
     * The session ID associated with the session token.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Token")
    FString SessionId = "";
};

USTRUCT(BlueprintType)
struct FInworldSave
{
    GENERATED_BODY()

    /**
     * Data array for saving in the Inworld environment.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Data")
    TArray<uint8> Data;
};

USTRUCT(BlueprintType)
struct FInworldEnvironment
{
    GENERATED_BODY()

    /**
     * The authentication URL for the Inworld environment.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Environment")
    FString AuthUrl = "";

    /**
     * The target URL for the Inworld environment.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Environment")
	FString TargetUrl = "";

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Multiplayer")
	int32 AudioPort = 0;
};

USTRUCT(BlueprintType)
struct FInworldAgentInfo
{
    GENERATED_BODY()

    /**
     * The name of the agent's brain.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Agent")
    FString BrainName = "";

    /**
     * The unique identifier of the agent.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Agent")
    FString AgentId = "";

    /**
     * The given name of the agent.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Agent")
    FString GivenName = "";
};

USTRUCT(BlueprintType)
struct FInworldConnectionErrorDetails
{
    GENERATED_BODY()

    /**
     * Type of connection error.
     */
    UPROPERTY(BlueprintReadOnly, Category = "Agent")
    EInworldConnectionErrorType ConnectionErrorType;

    /**
     * Type of reconnection attempt.
     */
    UPROPERTY(BlueprintReadOnly, Category = "Agent")
    EInworldReconnectionType ReconnectionType;

    /**
     * Time for the next reconnection attempt.
     */
    UPROPERTY(BlueprintReadOnly, Category = "Agent")
    int64 ReconnectTime = 0;

    /**
     * Maximum number of retry attempts.
     */
    UPROPERTY(BlueprintReadOnly, Category = "Agent")
    int32 MaxRetries = 0;
};

USTRUCT(BlueprintType)
struct FInworldAudioSessionOptions
{
    GENERATED_BODY()

    /**
     * The microphone mode for the audio session.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Audio")
    EInworldMicrophoneMode MicrophoneMode = EInworldMicrophoneMode::OPEN_MIC;

    /**
     * The understanding mode for the audio session.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Audio")
    EInworldUnderstandingMode UnderstandingMode = EInworldUnderstandingMode::FULL;

    /**
     * Equality operator for comparing two FInworldAudioSessionOptions instances.
     */
    bool operator==(const FInworldAudioSessionOptions& Other) const { return MicrophoneMode == Other.MicrophoneMode && UnderstandingMode == Other.UnderstandingMode; }

    /**
     * Inequality operator for comparing two FInworldAudioSessionOptions instances.
     */
    bool operator!=(const FInworldAudioSessionOptions& Other) const { return !(*this == Other); }
};

USTRUCT(BlueprintType)
struct FInworldPlayerSpeechOptions
{
    GENERATED_BODY()

    /**
     * Probability threshold for voice activity detection.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Speech")
    float VADProbThreshhold = 0.3f;
    /**
     * Number of buffer chunks for voice activity detection.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Speech")
    uint8 VADBufferChunksNum = 5;
    /**
     * Number of silence chunks for voice activity detection.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Speech")
    uint8 VADSilenceChunksNum = 5;
};

USTRUCT(BlueprintType)
struct FInworldEntityItem
{
    GENERATED_BODY()

    /**
     * The unique identifier of the entity item.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Entity")
    FString Id;

    /**
     * The display name of the entity item.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Entity")
    FString DisplayName;

    /**
     * The description of the entity item.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Entity")
    FString Description;

    /**
     * Additional properties of the entity item stored in a map.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Entity")
    TMap<FString, FString> Properties;
};
