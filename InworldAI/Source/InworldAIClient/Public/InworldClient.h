/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "InworldEnums.h"
#include "InworldTypes.h"
#include "InworldPackets.h"

#if !UE_BUILD_SHIPPING
#include "HAL/IConsoleManager.h"
#endif

#include "InworldClient.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInworldPacketReceived, const FInworldWrappedPacket&, WrappedPacket);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnInworldPacketReceivedCallback, const FInworldWrappedPacket&, WrappedPacket);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInworldPacketReceivedNative, const FInworldWrappedPacket& /*WrappedPacket*/);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInworldConnectionStateChanged, EInworldConnectionState, ConnectionState);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnInworldConnectionStateChangedCallback, EInworldConnectionState, ConnectionState);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInworldConnectionStateChangedNative, EInworldConnectionState /*ConnectionState*/);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInworldPerceivedLatency, FString, InteractionId, int32, LatencyMs);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnInworldPerceivedLatencyCallback, FString, InteractionId, int32, LatencyMs);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnInworldPerceivedLatencyNative, FString /*InteractionId*/, int32 /*ms*/);

DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnInworldSessionSavedCallback, FInworldSave, Save, bool, bSuccess);

UCLASS(BlueprintType)
class INWORLDAICLIENT_API UInworldClient : public UObject
{
public:
	GENERATED_BODY()

	UInworldClient();
	~UInworldClient();

	UFUNCTION(BlueprintCallable, Category = "Inworld|Session", meta = (AdvancedDisplay = "4", AutoCreateRefTerm = "PlayerProfile, Auth, Save, SessionToken, CapabilitySet"))
	void StartSession(const FString& SceneId, const FInworldPlayerProfile& PlayerProfile, const FInworldAuth& Auth, const FInworldSave& Save, const FInworldSessionToken& SessionToken, const FInworldCapabilitySet& CapabilitySet);
	UFUNCTION(BlueprintCallable, Category = "Inworld|Session")
	void StopSession();
	UFUNCTION(BlueprintCallable, Category = "Inworld|Session")
	void PauseSession();
	UFUNCTION(BlueprintCallable, Category = "Inworld|Session")
	void ResumeSession();

	UFUNCTION(BlueprintPure, Category = "Inworld|Session")
	FString GetSessionId() const;

	UFUNCTION(BlueprintCallable, Category = "Inworld|Session")
	void SaveSession(FOnInworldSessionSavedCallback Callback);

	UFUNCTION(BlueprintCallable, Category = "Inworld|Load|Character")
	void LoadCharacter(const FString& Id) { LoadCharacters({ Id }); }
	UFUNCTION(BlueprintCallable, Category = "Inworld|Load|Character")
	void LoadCharacters(const TArray<FString>& Ids);
	UFUNCTION(BlueprintCallable, Category = "Inworld|Load|Character")
	void UnloadCharacter(const FString& Id) { UnloadCharacters({ Id }); }
	UFUNCTION(BlueprintCallable, Category = "Inworld|Load|Character")
	void UnloadCharacters(const TArray<FString>& Ids);

	UFUNCTION(BlueprintCallable, Category = "Inworld|Load")
	void LoadSavedState(const FInworldSave& Save);
	UFUNCTION(BlueprintCallable, Category = "Inworld|Load")
	void LoadCapabilities(const FInworldCapabilitySet& CapabilitySet);
	UFUNCTION(BlueprintCallable, Category = "Inworld|Load")
	void LoadPlayerProfile(const FInworldPlayerProfile& PlayerProfile);

	UFUNCTION(BlueprintCallable, Category = "Message|Text")
	FInworldWrappedPacket SendTextMessage(const TArray<FString>& AgentIds, const FString& Text);

	UFUNCTION(BlueprintCallable, Category = "Message|Audio")
	void SendSoundMessage(const FString& AgentId, const TArray<uint8>& InputData, const TArray<uint8>& OutputData) { BroadcastSoundMessage({AgentId}, InputData, OutputData); }
	UFUNCTION(BlueprintCallable, Category = "Message|Audio")
	void BroadcastSoundMessage(const TArray<FString>& AgentIds, const TArray<uint8>& InputData, const TArray<uint8>& OutputData);
	UFUNCTION(BlueprintCallable, Category = "Message|Audio")
	void SendAudioSessionStart(const FString& AgentId) { BroadcastAudioSessionStart({ AgentId }); }
	UFUNCTION(BlueprintCallable, Category = "Message|Audio")
	void BroadcastAudioSessionStart(const TArray<FString>& AgentIds);
	UFUNCTION(BlueprintCallable, Category = "Message|Audio")
	void SendAudioSessionStop(const FString& AgentId) { BroadcastAudioSessionStop({ AgentId }); }
	UFUNCTION(BlueprintCallable, Category = "Message|Audio")
	void BroadcastAudioSessionStop(const TArray<FString>& AgentIds);

	UFUNCTION(BlueprintCallable, Category = "Message|Narration")
	void SendNarrationEvent(const FString& AgentId, const FString& Content);

	UFUNCTION(BlueprintCallable, Category = "Message|Trigger")
	void SendTrigger(FString AgentId, const FString& Name, const TMap<FString, FString>& Params) { BroadcastTrigger({AgentId}, Name, Params); }
	UFUNCTION(BlueprintCallable, Category = "Message|Trigger")
	void BroadcastTrigger(const TArray<FString>& AgentIds, const FString& Name, const TMap<FString, FString>& Params);

	UFUNCTION(BlueprintCallable, Category = "Message|Mutation")
	void SendChangeSceneEvent(const FString& SceneName);

	UFUNCTION(BlueprintCallable, Category = "Message|Mutation")
	void CancelResponse(const FString& AgentId, const FString& InteractionId, const TArray<FString>& UtteranceIds);

	UPROPERTY(BlueprintAssignable, Category = "Packet")
	FOnInworldPacketReceived OnPacketReceivedDelegate;
	FOnInworldPacketReceivedNative& OnPacketReceived() { return OnPacketReceivedDelegateNative; }

	UFUNCTION(BlueprintPure, Category = "Connection")
	EInworldConnectionState GetConnectionState() const;
	UFUNCTION(BlueprintPure, Category = "Connection")
	void GetConnectionError(FString& OutErrorMessage, int32& OutErrorCode) const;

	UPROPERTY(BlueprintAssignable, Category = "Connection")
	FOnInworldConnectionStateChanged OnConnectionStateChangedDelegate;
	FOnInworldConnectionStateChangedNative& OnConnectionStateChanged() { return OnConnectionStateChangedDelegateNative; }

	UPROPERTY(BlueprintAssignable, Category = "Connection")
	FOnInworldPerceivedLatency OnPerceivedLatencyDelegate;
	FOnInworldPerceivedLatencyNative& OnPerceivedLatency() { return OnPerceivedLatencyDelegateNative; }

	// Used for internal Inworld SDK testing - not documented, do not use.
	UFUNCTION(BlueprintCallable, Category = "Inworld Development")
	void SetEnvironment(const FInworldEnvironment& InEnvironment) { Environment = InEnvironment; }

private:
	FOnInworldPacketReceivedNative OnPacketReceivedDelegateNative;
	FOnInworldConnectionStateChangedNative OnConnectionStateChangedDelegateNative;
	FOnInworldPerceivedLatencyNative OnPerceivedLatencyDelegateNative;

	bool bIsBeingDestroyed = false;

#if !UE_BUILD_SHIPPING
	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnAudioDumperCVarChanged, bool /*Enabled*/, FString /*Path*/);
	static FOnAudioDumperCVarChanged OnAudioDumperCVarChanged;
	FDelegateHandle OnAudioDumperCVarChangedHandle;
	static FAutoConsoleVariableSink CVarSink;
	static void OnCVarsChanged();
#endif

	FInworldEnvironment Environment;
};