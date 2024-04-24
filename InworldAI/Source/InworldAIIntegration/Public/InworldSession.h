/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "InworldClient.h"
#include "InworldTypes.h"
#include "InworldPackets.h"
#include "InworldSession.generated.h"

class UInworldPlayer;
class UInworldCharacter;
class UInworldClient;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInworldCharactersInitialized, bool, bCharactersInitialized);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnInworldCharactersInitializedCallback, bool, bCharactersInitialized);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInworldCharactersInitializedNative, bool /*bCharactersInitialized*/);

UCLASS(BlueprintType)
class INWORLDAIINTEGRATION_API UInworldSession : public UObject
{
	GENERATED_BODY()
	
public:
	UInworldSession();
	virtual ~UInworldSession();

	UFUNCTION(BlueprintCallable, Category = "Inworld|Register")
	void RegisterCharacter(UInworldCharacter* Character);
	UFUNCTION(BlueprintCallable, Category = "Inworld|Register")
	void UnregisterCharacter(UInworldCharacter* Character);

	UFUNCTION(BlueprintPure, Category = "Inworld|Register")
	const TArray<UInworldCharacter*>& GetRegisteredCharacters() const { return RegisteredCharacters; }

	UFUNCTION(BlueprintCallable, Category = "Inworld|Session", meta = (AdvancedDisplay = "4", AutoCreateRefTerm = "PlayerProfile, Auth, Save, SessionToken, CapabilitySet"))
	void StartSession(const FString& SceneId, const FInworldPlayerProfile& PlayerProfile, const FInworldAuth& Auth, const FInworldSave& Save = FInworldSave(), const FInworldSessionToken& SessionToken = FInworldSessionToken(), const FInworldCapabilitySet& CapabilitySet = FInworldCapabilitySet());
	UFUNCTION(BlueprintCallable, Category = "Inworld|Session")
	void StopSession();
	UFUNCTION(BlueprintCallable, Category = "Inworld|Session")
	void PauseSession() { InworldClient->PauseSession(); }
	UFUNCTION(BlueprintCallable, Category = "Inworld|Session")
	void ResumeSession() { InworldClient->ResumeSession(); }

	UFUNCTION(BlueprintPure, Category = "Inworld|Session")
	FString GetSessionId() const { return InworldClient->GetSessionId(); }

	UFUNCTION(BlueprintCallable, Category = "Inworld|Session")
	void SaveSession(FOnInworldSessionSavedCallback Callback) { InworldClient->SaveSession(Callback); }

	UFUNCTION(BlueprintCallable, Category = "Inworld|Load|Character")
	void LoadCharacter(UInworldCharacter* Character) { LoadCharacters({ Character }); }
	UFUNCTION(BlueprintCallable, Category = "Inworld|Load|Character")
	void LoadCharacters(const TArray<UInworldCharacter*>& Characters);
	UFUNCTION(BlueprintCallable, Category = "Inworld|Load|Character")
	void UnloadCharacter(UInworldCharacter* Character) { UnloadCharacters({ Character }); }
	UFUNCTION(BlueprintCallable, Category = "Inworld|Load|Character")
	void UnloadCharacters(const TArray<UInworldCharacter*>& Characters);

	UFUNCTION(BlueprintCallable, Category = "Inworld|Load")
	void LoadSavedState(const FInworldSave& Save) { InworldClient->LoadSavedState(Save); }
	UFUNCTION(BlueprintCallable, Category = "Inworld|Load")
	void LoadCapabilities(const FInworldCapabilitySet& CapabilitySet) { InworldClient->LoadCapabilities(CapabilitySet); }
	UFUNCTION(BlueprintCallable, Category = "Inworld|Load")
	void LoadPlayerProfile(const FInworldPlayerProfile& PlayerProfile) { InworldClient->LoadPlayerProfile(PlayerProfile); }

	UFUNCTION(BlueprintCallable, Category = "Message|Text")
	FInworldWrappedPacket SendTextMessage(UInworldCharacter* Character, const FString& Message) { return BroadcastTextMessage({Character}, Message); }
	UFUNCTION(BlueprintCallable, Category = "Message|Text")
	FInworldWrappedPacket BroadcastTextMessage(const TArray<UInworldCharacter*>& Characters, const FString& Message);

	UFUNCTION(BlueprintCallable, Category = "Message|Audio")
	void SendSoundMessage(UInworldCharacter* Character, const TArray<uint8>& InputData, const TArray<uint8>& OutputData) { BroadcastSoundMessage({ Character }, InputData, OutputData); }
	UFUNCTION(BlueprintCallable, Category = "Message|Audio")
	void BroadcastSoundMessage(const TArray<UInworldCharacter*>& Characters, const TArray<uint8>& InputData, const TArray<uint8>& OutputData);
	UFUNCTION(BlueprintCallable, Category = "Message|Audio")
	void SendAudioSessionStart(UInworldCharacter* Character) { BroadcastAudioSessionStart({ Character }); }
	UFUNCTION(BlueprintCallable, Category = "Message|Audio")
	void BroadcastAudioSessionStart(const TArray<UInworldCharacter*>& Characters);
	UFUNCTION(BlueprintCallable, Category = "Message|Audio")
	void SendAudioSessionStop(UInworldCharacter* Character) { BroadcastAudioSessionStop({ Character }); }
	UFUNCTION(BlueprintCallable, Category = "Message|Audio")
	void BroadcastAudioSessionStop(const TArray<UInworldCharacter*>& Characters);

	UFUNCTION(BlueprintCallable, Category = "Message|Narration")
	void SendNarrationEvent(UInworldCharacter* Character, const FString& Content);

	UFUNCTION(BlueprintCallable, Category = "Message|Trigger")
	void SendTrigger(UInworldCharacter* Character, const FString& Name, const TMap<FString, FString>& Params) { BroadcastTrigger({ Character }, Name, Params); }
	UFUNCTION(BlueprintCallable, Category = "Message|Trigger")
	void BroadcastTrigger(const TArray<UInworldCharacter*>& Characters, const FString& Name, const TMap<FString, FString>& Params);

	UFUNCTION(BlueprintCallable, Category = "Message|Mutation")
	void SendChangeSceneEvent(const FString& SceneName);

	UFUNCTION(BlueprintCallable, Category = "Message|Mutation")
	void CancelResponse(UInworldCharacter* Character, const FString& InteractionId, const TArray<FString>& UtteranceIds);

	UPROPERTY(BlueprintAssignable, Category = "Packet")
	FOnInworldPacketReceived OnPacketReceivedDelegate;
	FOnInworldPacketReceivedNative& OnPacketReceived() { return OnPacketReceivedDelegateNative; }

	UFUNCTION(BlueprintPure, Category = "Connection")
	EInworldConnectionState GetConnectionState() const { return InworldClient->GetConnectionState(); }
	UFUNCTION(BlueprintPure, Category = "Connection")
	void GetConnectionError(FString& OutErrorMessage, int32& OutErrorCode) const { InworldClient->GetConnectionError(OutErrorMessage, OutErrorCode); }

	UPROPERTY(BlueprintAssignable, Category = "Connection")
	FOnInworldConnectionStateChanged OnConnectionStateChangedDelegate;
	FOnInworldConnectionStateChangedNative& OnConnectionStateChanged() { return OnConnectionStateChangedDelegateNative; }

	UPROPERTY(BlueprintAssignable, Category = "Connection")
	FOnInworldCharactersInitialized OnCharactersInitializedDelegate;
	FOnInworldCharactersInitializedNative& OnCharactersInitialized() { return OnCharactersInitializedDelegateNative; }

	UPROPERTY(BlueprintAssignable, Category = "Connection")
	FOnInworldPerceivedLatency OnPerceivedLatencyDelegate;
	FOnInworldPerceivedLatencyNative& OnPerceivedLatency() { return OnPerceivedLatencyDelegateNative; }

private:
	void PossessAgents(const TArray<FInworldAgentInfo>& AgentInfos);
	void UnpossessAgents();

private:
	TArray<UInworldCharacter*> RegisteredCharacters;
	TMap<FString, UInworldCharacter*> BrainNameToCharacter;
	TMap<FString, UInworldCharacter*> AgentIdToCharacter;
	TMap<FString, FInworldAgentInfo> BrainNameToAgentInfo;

	bool bCharactersInitialized;

	UPROPERTY()
	UInworldClient* InworldClient;

	FDelegateHandle OnClientPacketReceivedHandle;
	FDelegateHandle OnClientConnectionStateChangedHandle;
	FDelegateHandle OnClientPerceivedLatencyHandle;

	FOnInworldPacketReceivedNative OnPacketReceivedDelegateNative;
	FOnInworldConnectionStateChangedNative OnConnectionStateChangedDelegateNative;
	FOnInworldCharactersInitializedNative OnCharactersInitializedDelegateNative;
	FOnInworldPerceivedLatencyNative OnPerceivedLatencyDelegateNative;

	// Temp: Hack until deprecated functions are removed
	friend class UInworldApiSubsystem;
};

UINTERFACE(MinimalAPI, BlueprintType)
class UInworldSessionOwnerInterface : public UInterface
{
	GENERATED_BODY()
};

class INWORLDAIINTEGRATION_API IInworldSessionOwnerInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inworld")
	UInworldSession* GetInworldSession() const;
};
