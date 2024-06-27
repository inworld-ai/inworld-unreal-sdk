/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "InworldAudioSender.h"
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

	UFUNCTION(BlueprintCallable, Category = "Session", meta = (AdvancedDisplay = "4", AutoCreateRefTerm = "PlayerProfile, Auth, Save, SessionToken, CapabilitySet"))
	void StartSession(const FString& SceneId, const FInworldPlayerProfile& PlayerProfile, const FInworldAuth& Auth, const FInworldSave& Save, const FInworldSessionToken& SessionToken, const FInworldCapabilitySet& CapabilitySet);
	UFUNCTION(BlueprintCallable, Category = "Session")
	void StopSession();
	UFUNCTION(BlueprintCallable, Category = "Session")
	void PauseSession();
	UFUNCTION(BlueprintCallable, Category = "Session")
	void ResumeSession();

	UFUNCTION(BlueprintPure, Category = "Session")
	FString GetSessionId() const;

	UFUNCTION(BlueprintCallable, Category = "Session")
	void SaveSession(FOnInworldSessionSavedCallback Callback);

	UFUNCTION(BlueprintCallable, Category = "Session")
	void SendInteractionFeedback(const FString& InteractionId, bool bIsLike, const FString& Message);

	UFUNCTION(BlueprintCallable, Category = "Load|Character")
	void LoadCharacter(const FString& Id) { LoadCharacters({ Id }); }
	UFUNCTION(BlueprintCallable, Category = "Load|Character")
	void LoadCharacters(const TArray<FString>& Ids);
	UFUNCTION(BlueprintCallable, Category = "Load|Character")
	void UnloadCharacter(const FString& Id) { UnloadCharacters({ Id }); }
	UFUNCTION(BlueprintCallable, Category = "Load|Character")
	void UnloadCharacters(const TArray<FString>& Ids);

	UFUNCTION(BlueprintCallable, Category = "Load")
	void LoadSavedState(const FInworldSave& Save);
	UFUNCTION(BlueprintCallable, Category = "Load")
	void LoadCapabilities(const FInworldCapabilitySet& CapabilitySet);
	UFUNCTION(BlueprintCallable, Category = "Load")
	void LoadPlayerProfile(const FInworldPlayerProfile& PlayerProfile);

	UFUNCTION(BlueprintCallable, Category = "Conversation")
	FString UpdateConversation(const FString& ConversationId, const TArray<FString>& AgentIds, bool bIncludePlayer);

	UFUNCTION(BlueprintCallable, Category = "Message|Text")
	FInworldWrappedPacket SendTextMessage(const FString& AgentId, const FString& Text);
	UFUNCTION(BlueprintCallable, Category = "Message|Text")
	FInworldWrappedPacket SendTextMessageToConversation(const FString& ConversationId, const FString& Text);

	UFUNCTION(BlueprintCallable, Category = "Message|Audio")
	void SendSoundMessage(const FString& AgentId, const TArray<uint8>& InputData, const TArray<uint8>& OutputData);
	UFUNCTION(BlueprintCallable, Category = "Message|Audio")
	void SendSoundMessageToConversation(const FString& ConversationId, const TArray<uint8>& InputData, const TArray<uint8>& OutputData);

	UFUNCTION(BlueprintCallable, Category = "Message|Audio")
	void SendAudioSessionStart(const FString& AgentId, UObject* Owner, EInworldMicrophoneMode MicrophoneMode = EInworldMicrophoneMode::OPEN_MIC);
	UFUNCTION(BlueprintCallable, Category = "Message|Audio")
	void SendAudioSessionStartToConversation(const FString& ConversationId, UObject* Owner, EInworldMicrophoneMode MicrophoneMode = EInworldMicrophoneMode::OPEN_MIC);

	UFUNCTION(BlueprintCallable, Category = "Message|Audio")
	void SendAudioSessionStop(const FString& AgentId);
	UFUNCTION(BlueprintCallable, Category = "Message|Audio")
	void SendAudioSessionStopToConversation(const FString& ConversationId);

	UFUNCTION(BlueprintCallable, Category = "Message|Narration")
	void SendNarrationEvent(const FString& AgentId, const FString& Content);

	UFUNCTION(BlueprintCallable, Category = "Message|Trigger")
	void SendTrigger(const FString& AgentId, const FString& Name, const TMap<FString, FString>& Params);
	UFUNCTION(BlueprintCallable, Category = "Message|Trigger")
	void SendTriggerToConversation(const FString& ConversationId, const FString& Name, const TMap<FString, FString>& Params);

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
	void GetConnectionError(FString& OutErrorMessage, int32& OutErrorCode, FInworldConnectionErrorDetails& OutErrorDetails) const;

	UPROPERTY(BlueprintAssignable, Category = "Connection")
	FOnInworldConnectionStateChanged OnConnectionStateChangedDelegate;
	FOnInworldConnectionStateChangedNative& OnConnectionStateChanged() { return OnConnectionStateChangedDelegateNative; }

	UPROPERTY(BlueprintAssignable, Category = "Connection")
	FOnInworldPerceivedLatency OnPerceivedLatencyDelegate;
	FOnInworldPerceivedLatencyNative& OnPerceivedLatency() { return OnPerceivedLatencyDelegateNative; }

	// Used for internal Inworld SDK testing - not documented, do not use.
	UFUNCTION(BlueprintCallable, Category = "Inworld Development")
	void SetEnvironment(const FInworldEnvironment& InEnvironment) { Environment = InEnvironment; }

	FOnInworldVADNative& OnVAD() { return OnVADDelegateNative; }

private:
	UPROPERTY()
	UInworldAudioSender* AudioSender;
	
	FOnInworldPacketReceivedNative OnPacketReceivedDelegateNative;
	FOnInworldConnectionStateChangedNative OnConnectionStateChangedDelegateNative;
	FOnInworldPerceivedLatencyNative OnPerceivedLatencyDelegateNative;
	FOnInworldVADNative OnVADDelegateNative;

	FDelegateHandle OnVADHandle;

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