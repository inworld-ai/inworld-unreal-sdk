/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once


#include "CoreMinimal.h"
#include "InworldClient.h"
#include "Subsystems/WorldSubsystem.h"
#include "InworldState.h"
#include "InworldComponentInterface.h"
#include "NDK/Client.h"
#include "InworldApi.generated.h"

namespace Inworld
{
	class ICharacterComponent;
	class IPlayerComponent;
}
class USoundWave;
class UInworldAudioRepl;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnConnectionStateChanged, EInworldConnectionState, State);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCustomTrigger, FString, Name);

UCLASS(BlueprintType, Config = Engine)
class INWORLDAIINTEGRATION_API UInworldApiSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
    UInworldApiSubsystem();

    /**
     * Start InworldAI session
     * call after all Player/Character components have been registered
     * @param SceneName : full inworld studio scene name
     * @param Token : optional, will be generated if empty
     * @param SessionId : optional, will be generated if empty
     */
    UFUNCTION(BlueprintCallable, Category = "Inworld", meta = (AdvancedDisplay = "4"))
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
     * Register Character component
     * call before StartSession
     */
    void RegisterCharacterComponent(Inworld::ICharacterComponent* Component);
    void UnregisterCharacterComponent(Inworld::ICharacterComponent* Component);

    bool IsCharacterComponentRegistered(Inworld::ICharacterComponent* Component);

	void UpdateCharacterComponentRegistrationOnClient(Inworld::ICharacterComponent* Component, const FString& NewAgentId, const FString& OldAgentId);

public:
    /**
     * Register Player component
     * call before StartSession
     */
	void RegisterPlayerComponent(Inworld::IPlayerComponent* Component);
	void UnregisterPlayerComponent();

    /** Send text to agent */
	UFUNCTION(BlueprintCallable, Category = "Messages")
    void SendTextMessage(const FString& AgentId, const FString& Text);

    /** Send trigger to agent */
	UFUNCTION(BlueprintCallable, Category = "Messages")
	void SendTrigger(FString AgentId, const FString& Name);
    [[deprecated("UInworldApiSubsystem::SendCustomEvent is deprecated, please use UInworldApiSubsystem::SendTrigger")]]
    void SendCustomEvent(FString AgentId, const FString& Name) { SendTrigger(AgentId, Name); }

    /**
     * Send audio to agent
     * start audio session before sending audio
     * stop audio session after all audio chunks have been sent
     * chunks should be ~100ms
     */
    UFUNCTION(BlueprintCallable, Category = "Messages")
	void SendAudioMessage(const FString& AgentId, USoundWave* SoundWave);
	
    void SendAudioDataMessage(const FString& AgentId, const std::string& Data);
	void SendAudioDataMessageWithAEC(const FString& AgentId, USoundWave* InputWave, USoundWave* OutputWave);
	void SendAudioDataMessageWithAEC(const FString& AgentId, const std::vector<int16_t>& InputData, const std::vector<int16_t>& OutputData);
    
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

    /** Get current connection state */
    UFUNCTION(BlueprintCallable, Category = "Connection")
	EInworldConnectionState GetConnectionState() const { return static_cast<EInworldConnectionState>(Client->GetConnectionState()); }

    /** Get connection error message and code from previous Disconnect */
    UFUNCTION(BlueprintCallable, Category = "Inworld")
    void GetConnectionError(FString& Message, int32& Code);
    
    /** Get all registered character components */
	const TArray<Inworld::ICharacterComponent*>& GetCharacterComponents() const { return CharacterComponentRegistry; }

    /** Get registered character component by agent id */
    Inworld::ICharacterComponent* GetCharacterComponentByAgentId(const FString& AgentId) const;

    /** Cancel agents response in case agent has been interrupted by player */
    UFUNCTION(BlueprintCallable, Category = "Messages")
    void CancelResponse(const FString& AgentId, const FString& InteractionId, const TArray<FString>& UtteranceIds);
    void CancelResponse(const FString& AgentId, const std::string& InteractionId, const std::vector<std::string>& UtteranceIds);

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
    void HandleAudioEventOnClient(std::shared_ptr<FInworldAudioDataEvent> Packet);

    UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "EventDispatchers")
    FOnConnectionStateChanged OnConnectionStateChanged;

    UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "EventDispatchers")
    FCustomTrigger OnCustomTrigger;

private:
	void DispatchPacket(std::shared_ptr<FInworldPacket> InworldPacket);

    UPROPERTY(EditAnywhere, config, Category = "Connection")
    FString SentryDSN;

	UPROPERTY(EditAnywhere, config, Category = "Connection")
	FString SentryTransactionName;

	UPROPERTY(EditAnywhere, config, Category = "Connection")
	FString SentryTransactionOperation;

    UPROPERTY(EditAnywhere, config, Category = "Connection")
    float RetryConnectionIntervalTime = 0.25f;

    UPROPERTY(EditAnywhere, config, Category = "Connection")
    float MaxRetryConnectionTime = 5.0f;

    float CurrentRetryConnectionTime = 1.0f;

    UPROPERTY()
    UInworldAudioRepl* AudioRepl;

    FTimerHandle RetryConnectionTimerHandle;

    TMap<FString, Inworld::ICharacterComponent*> CharacterComponentByBrainName;
    TMap<FString, Inworld::ICharacterComponent*> CharacterComponentByAgentId;
    TArray<Inworld::ICharacterComponent*> CharacterComponentRegistry;
    TMap<FString, Inworld::AgentInfo> AgentInfoByBrain;

    Inworld::IPlayerComponent* PlayerComponent;

    Inworld::FClient::SharedPtr Client;

    bool bCharactersInitialized = false;
};