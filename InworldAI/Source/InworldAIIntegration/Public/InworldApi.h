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
    UFUNCTION(BlueprintPure, Category = "Session")
    UInworldSession* GetInworldSession();

    /**
     * Start InworldAI session
     * call after all Player/Character components have been registered
     * @param SceneName : full inworld studio scene name
     * @param Token : optional, will be generated if empty
     * @param SessionId : optional, will be generated if empty
     */
    UFUNCTION(BlueprintCallable, Category = "Inworld", meta = (DisplayName = "StartSession", AdvancedDisplay = "4", AutoCreateRefTerm = "PlayerProfile, Capabilities, Auth, SessionToken, Environment"))
    void StartSession_V2(const FString& SceneName, const FInworldPlayerProfile& PlayerProfile, const FInworldCapabilitySet& Capabilities, const FInworldAuth& Auth, const FInworldSessionToken& SessionToken, const FInworldEnvironment& Environment, FString UniqueUserIdOverride, FInworldSave SavedSessionState);

    UFUNCTION(BlueprintCallable, Category = "Inworld", meta = (AdvancedDisplay = "4", DeprecatedFunction, DeprecationMessage = "Please recreate 'Start Session' node with updated parameters."))
    void StartSession(const FString& SceneName, const FString& PlayerName, const FString& ApiKey, const FString& ApiSecret, const FString& AuthUrlOverride = "", const FString& TargetUrlOverride = "", const FString& Token = "", int64 TokenExpiration = 0, const FString& SessionId = "");

    /**
     * Pause InworldAI session
     * call after StartSession has been called to pause
     */
    UFUNCTION(BlueprintCallable, Category = "Inworld")
    void PauseSession();

    /**
     * Resume InworldAI session
     * call after PauseSession has been called to resume
     */
    UFUNCTION(BlueprintCallable, Category = "Inworld")
    void ResumeSession();

    /**
     * Stop InworldAI session
     * call after StartSession has been called to stop
     */
    UFUNCTION(BlueprintCallable, Category = "Inworld")
    void StopSession();

    /**
     * Save InworldAI session data
     */
    UFUNCTION(BlueprintCallable, Category = "Inworld")
    void SaveSession(FOnInworldSessionSavedCallback Delegate);

    /**
     * Set delegate for response latency tracker
     */
    UFUNCTION(BlueprintCallable, Category = "Inworld")
    void SetResponseLatencyTrackerDelegate(const FOnInworldPerceivedLatencyCallback& Delegate);

    /**
     * Clear delegate for response latency tracker
     */
    UFUNCTION(BlueprintCallable, Category = "Inworld")
    void ClearResponseLatencyTrackerDelegate(const FOnInworldPerceivedLatencyCallback& Delegate);

    /**
	 * Load new characters
	 */
    UFUNCTION(BlueprintCallable, Category = "Inworld")
    void LoadCharacters(const TArray<FString>& Names);

    /**
     * Unload characters
     */
    UFUNCTION(BlueprintCallable, Category = "Inworld")
    void UnloadCharacters(const TArray<FString>& Names);

    /**
     * Load saved state
     */
    UFUNCTION(BlueprintCallable, Category = "Inworld")
    void LoadSavedState(const FInworldSave& SavedState);

	/**
	 * Load capabilities
	 */
	UFUNCTION(BlueprintCallable, Category = "Inworld")
	void LoadCapabilities(const FInworldCapabilitySet& Capabilities);

	/**
	 * Load player profile
	 */
	UFUNCTION(BlueprintCallable, Category = "Inworld")
	void LoadPlayerProfile(const FInworldPlayerProfile& PlayerProfile);

public:
    /** Send text to agent */
	UFUNCTION(BlueprintCallable, Category = "Messages")
    void SendTextMessage(const FString& AgentId, const FString& Text);

    /** Send trigger to agent */
	UFUNCTION(BlueprintCallable, Category = "Messages", meta = (AutoCreateRefTerm = "Params"))
	void SendTrigger(const FString& AgentId, const FString& Name, const TMap<FString, FString>& Params);
    [[deprecated("UInworldApiSubsystem::SendCustomEvent is deprecated, please use UInworldApiSubsystem::SendTrigger")]]
    void SendCustomEvent(const FString& AgentId, const FString& Name) { SendTrigger(AgentId, Name, {}); }

    /** Send narration to agent */
	UFUNCTION(BlueprintCallable, Category = "Messages")
	void SendNarrationEvent(const FString& AgentId, const FString& Content);

    /**
     * Send audio to agent
     * start audio session before sending audio
     * stop audio session after all audio chunks have been sent
     * chunks should be ~100ms
     */
    UFUNCTION(BlueprintCallable, Category = "Messages")
    void SendAudioMessage(const FString& AgentId, const TArray<uint8>& InputData, const TArray<uint8>& OutputData);
    
    /**
     * Start audio session with agent
     * call before sending audio messages
     */
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StartAudioSession(const FString& AgentId);

    /**
     * Stop audio session with agent
     * call after all audio messages have been sent
     */
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopAudioSession(const FString& AgentId);

    /** Change scene */
    UFUNCTION(BlueprintCallable, Category = "Messages")
    void ChangeScene(const FString& SceneId);

    /** Get current connection state */
    UFUNCTION(BlueprintCallable, Category = "Connection")
    EInworldConnectionState GetConnectionState() const;

    /** Get connection error message and code from previous Disconnect */
    UFUNCTION(BlueprintCallable, Category = "Inworld")
    void GetConnectionError(FString& Message, int32& Code, FInworldConnectionErrorDetails& Details);

    /** Cancel agents response in case agent has been interrupted by player */
    UFUNCTION(BlueprintCallable, Category = "Messages")
    void CancelResponse(const FString& AgentId, const FString& InteractionId, const TArray<FString>& UtteranceIds);
    /**
    * Call on Inworld::FCustomEvent coming to agent
    * custom events meant to be triggered on interaction end (see InworldCharacterComponent)
    */
    void NotifyCustomTrigger(const FString& Name) { OnCustomTrigger.Broadcast(Name); }

	/** 
    * Call this in multiplayer on BeginPlay both on server and client
    * called in UE5 automatically
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

    UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "EventDispatchers", meta = (DeprecatedProperty, DeprecationMessage = "Use InworldSession->OnConnectionStateChanged."))
    FOnConnectionStateChanged OnConnectionStateChanged;

    UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "EventDispatchers", meta = (DeprecatedProperty, DeprecationMessage = "Use InworldSession->OnCharactersInitialized."))
    FOnCharactersInitialized OnCharactersInitialized;

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
