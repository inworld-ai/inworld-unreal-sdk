/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Runtime/Launch/Resources/Version.h"

#include "InworldClient.h"
#include "InworldSession.h"
#include "InworldEnums.h"
#include "InworldTypes.h"
#include "InworldPackets.h"

#include "InworldApi.generated.h"

class USoundWave;
class UInworldAudioRepl;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnConnectionStateChanged, EInworldConnectionState, State);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharactersInitialized, bool, bCharactersInitialized);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCustomTrigger, FString, Name);

UCLASS(BlueprintType, Config = InworldAI)
class INWORLDAIINTEGRATION_API UInworldApiSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
    UInworldApiSubsystem();

    void SetInworldSession(UInworldSession* Session);
    
    /**
     * Get the current Inworld session.
     * @return The current Inworld session.
     */
    UFUNCTION(BlueprintPure, Category = "Session")
    UInworldSession* GetInworldSession();

    /**
     * Start InworldAI session
     * Call after all Player/Character components have been registered
     * @param SceneName Full Inworld studio scene name
     * @param PlayerProfile Player profile data
     * @param Capabilities Player capabilities data
     * @param Auth Authentication data
     * @param SessionToken Session token data
     * @param Environment Environment settings data
     * @param UniqueUserIdOverride Override for unique user ID
     * @param SavedSessionState Saved session state data
     */
    UFUNCTION(BlueprintCallable, Category = "Inworld", meta = (DisplayName = "StartSession", AdvancedDisplay = "4", AutoCreateRefTerm = "PlayerProfile, Capabilities, Auth, SessionToken, Environment"))
    void StartSession_V2(const FString& SceneName, const FInworldPlayerProfile& PlayerProfile, const FInworldCapabilitySet& Capabilities, const FInworldAuth& Auth, const FInworldSessionToken& SessionToken, const FInworldEnvironment& Environment, FString UniqueUserIdOverride, FInworldSave SavedSessionState);

    /**
     * Start an Inworld session with the specified parameters.
     * @param SceneName Full Inworld studio scene name
     * @param PlayerName Name of the player
     * @param ApiKey API key for authentication
     * @param ApiSecret API secret for authentication
     * @param AuthUrlOverride Optional override for authentication URL
     * @param TargetUrlOverride Optional override for target URL
     * @param Token Authentication token
     * @param TokenExpiration Token expiration time
     * @param SessionId Session ID
     */
    UFUNCTION(BlueprintCallable, Category = "Inworld", meta = (AdvancedDisplay = "4", DeprecatedFunction, DeprecationMessage = "Please recreate 'Start Session' node with updated parameters."))
    void StartSession(const FString& SceneName, const FString& PlayerName, const FString& ApiKey, const FString& ApiSecret, const FString& AuthUrlOverride = "", const FString& TargetUrlOverride = "", const FString& Token = "", int64 TokenExpiration = 0, const FString& SessionId = "");

    /**
     * Pause InworldAI session
     * Call after StartSession has been called to pause
     */
    UFUNCTION(BlueprintCallable, Category = "Inworld")
    void PauseSession();

    /**
     * Resume InworldAI session
     * Call after PauseSession has been called to resume
     */
    UFUNCTION(BlueprintCallable, Category = "Inworld")
    void ResumeSession();

    /**
     * Stop InworldAI session
     * Call after StartSession has been called to stop
     */
    UFUNCTION(BlueprintCallable, Category = "Inworld")
    void StopSession();

    /**
     * Save the InworldAI session data.
     * @param Delegate The callback to execute after saving the session.
     */
    UFUNCTION(BlueprintCallable, Category = "Inworld")
    void SaveSession(FOnInworldSessionSavedCallback Delegate);

    /**
     * Set the delegate for the response latency tracker.
     * @param Delegate The delegate for tracking perceived latency.
     */
    UFUNCTION(BlueprintCallable, Category = "Inworld")
    void SetResponseLatencyTrackerDelegate(const FOnInworldPerceivedLatencyCallback& Delegate);

    /**
     * Clear the delegate for the response latency tracker.
     * @param Delegate The delegate to clear for tracking perceived latency.
     */
    UFUNCTION(BlueprintCallable, Category = "Inworld")
    void ClearResponseLatencyTrackerDelegate(const FOnInworldPerceivedLatencyCallback& Delegate);

    /**
     * Load new characters
     * @param Names Array of character names to load
     */
    UFUNCTION(BlueprintCallable, Category = "Inworld")
    void LoadCharacters(const TArray<FString>& Names);

    /**
     * Unload characters
     * @param Names Array of character names to unload
     */
    UFUNCTION(BlueprintCallable, Category = "Inworld")
    void UnloadCharacters(const TArray<FString>& Names);

public:
    /** Send text to agent
     * @param AgentId ID of the agent
     * @param Text Text message to send
     */
    UFUNCTION(BlueprintCallable, Category = "Messages")
    void SendTextMessage(const FString& AgentId, const FString& Text);

    /** Send trigger to agent
     * @param AgentId ID of the agent
     * @param Name Name of the trigger
     * @param Params Additional parameters for the trigger
     */
    UFUNCTION(BlueprintCallable, Category = "Messages", meta = (AutoCreateRefTerm = "Params"))
    void SendTrigger(const FString& AgentId, const FString& Name, const TMap<FString, FString>& Params);
    [[deprecated("UInworldApiSubsystem::SendCustomEvent is deprecated, please use UInworldApiSubsystem::SendTrigger")]]
    void SendCustomEvent(const FString& AgentId, const FString& Name) { SendTrigger(AgentId, Name, {}); }

    /** Send narration to agent
     * @param AgentId ID of the agent
     * @param Content Narration content to send
     */
    UFUNCTION(BlueprintCallable, Category = "Messages")
    void SendNarrationEvent(const FString& AgentId, const FString& Content);

    /**
     * Send audio to agent
     * Start audio session before sending audio
     * Stop audio session after all audio chunks have been sent
     * Chunks should be around 100ms
     * @param AgentId ID of the agent
     * @param InputData Input audio data
     * @param OutputData Output audio data
     */
    UFUNCTION(BlueprintCallable, Category = "Messages", meta = (DeprecatedFunction))
    void SendAudioMessage(const FString& AgentId, const TArray<uint8>& InputData, const TArray<uint8>& OutputData);
    
    /**
     * Start audio session with agent
     * Call before sending audio messages
     * @param AgentId ID of the agent
     * @param SessionOptions Options for the audio session
     */
    UFUNCTION(BlueprintCallable, Category = "Audio", meta = (DeprecatedFunction))
    void StartAudioSession(const FString& AgentId, FInworldAudioSessionOptions SessionOptions);

    /**
     * Stop audio session with agent
     * Call after all audio messages have been sent
     * @param AgentId ID of the agent
     */
    UFUNCTION(BlueprintCallable, Category = "Audio", meta = (DeprecatedFunction))
    void StopAudioSession(const FString& AgentId);

    /** Change scene
     * @param SceneId ID of the scene to change to
     */
    UFUNCTION(BlueprintCallable, Category = "Messages")
    void ChangeScene(const FString& SceneId);

    /**
     * Get the current connection state.
     * @return The current connection state.
     */
    UFUNCTION(BlueprintCallable, Category = "Connection")
    EInworldConnectionState GetConnectionState() const;

    /** Get connection error message and code from previous Disconnect
     * @param Message Output parameter for the error message
     * @param Code Output parameter for the error code
     * @param OutErrorDetails Additional error details
     */
    UFUNCTION(BlueprintCallable, Category = "Inworld")
    void GetConnectionError(FString& Message, int32& Code, FInworldConnectionErrorDetails& OutErrorDetails);

    /** Cancel agents response in case agent has been interrupted by player
     * @param AgentId ID of the agent
     * @param InteractionId ID of the interaction
     * @param UtteranceIds Array of utterance IDs
     */
    UFUNCTION(BlueprintCallable, Category = "Messages")
    void CancelResponse(const FString& AgentId, const FString& InteractionId, const TArray<FString>& UtteranceIds);
    /**
     * Call on Inworld::FCustomEvent coming to agent
     * Custom events meant to be triggered on interaction end (see InworldCharacterComponent)
     * @param Name Name of the custom trigger
     */
    void NotifyCustomTrigger(const FString& Name) { OnCustomTrigger.Broadcast(Name); }

    /** 
    * Call this in multiplayer on BeginPlay both on server and client
    * Called in UE5 automatically
    */
    UFUNCTION(BlueprintCallable, Category = "Multiplayer")
    void StartAudioReplication();

    /** Subsystem interface */
    virtual bool DoesSupportWorldType(EWorldType::Type WorldType) const override;
	   virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	   virtual void Deinitialize() override;
#if ENGINE_MAJOR_VERSION > 4
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;
#endif

	   void ReplicateAudioEventFromServer(FInworldAudioDataEvent& Packet);
    void HandleAudioEventOnClient(TSharedPtr<FInworldAudioDataEvent> Packet);

    /**
     * Event dispatcher for when the connection state changes. (Deprecated, use InworldSession->OnConnectionStateChanged.)
     */
    UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "EventDispatchers", meta = (DeprecatedProperty, DeprecationMessage = "Use InworldSession->OnConnectionStateChanged."))
    FOnConnectionStateChanged OnConnectionStateChanged;

    /**
     * Event dispatcher for when characters are initialized. (Deprecated, use InworldSession->OnCharactersInitialized.)
     */
    UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "EventDispatchers", meta = (DeprecatedProperty, DeprecationMessage = "Use InworldSession->OnCharactersInitialized."))
    FOnCharactersInitialized OnCharactersInitialized;

    /**
     * Event dispatcher for custom triggers. (Deprecated, use InworldCharacter->OnTrigger.)
     */
    UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "EventDispatchers", meta = (DeprecatedProperty, DeprecationMessage = "Use InworldCharacter->OnTrigger."))
    FCustomTrigger OnCustomTrigger;

private:
    UPROPERTY()
    UInworldAudioRepl* AudioRepl;

    UPROPERTY()
    UInworldSession* InworldSession;

#if defined(WITH_GAMEPLAY_DEBUGGER) && WITH_GAMEPLAY_DEBUGGER
	friend class FInworldGameplayDebuggerCategory;
#endif
};
