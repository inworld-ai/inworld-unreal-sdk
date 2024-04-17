/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "InworldEnums.h"
#include "InworldPackets.h"

#if !UE_BUILD_SHIPPING
#include "HAL/IConsoleManager.h"
#endif

#include "InworldClient.generated.h"

DECLARE_DELEGATE_OneParam(FOnInworldSceneLoaded, TArray<FInworldAgentInfo>);
DECLARE_DELEGATE_TwoParams(FOnInworldSessionSaved, FInworldSave, bool);
DECLARE_DELEGATE_TwoParams(FOnInworldLatency, FString, int32)
DECLARE_DELEGATE_OneParam(FOnInworldConnectionStateChanged, EInworldConnectionState);
DECLARE_DELEGATE_OneParam(FOnInworldPacketReceived, TSharedPtr<FInworldPacket>);

USTRUCT()
struct INWORLDAICLIENT_API FInworldClient
{
public:
	GENERATED_BODY()

	void Init();
	void Destroy();

	void Start(const FString& SceneName, const FInworldPlayerProfile& PlayerProfile, const FInworldCapabilitySet& Capabilities, const FInworldAuth& Auth, const FInworldSessionToken& SessionToken, const FInworldSave& Save, const FInworldEnvironment& Environment);

	void Stop();

	void Pause();
	void Resume();

	void SaveSession();

	void LoadCharacters(const TArray<FString>& Names);
	void UnloadCharacters(const TArray<FString>& Names);
	void LoadSavedState(const TArray<uint8>& SavedState);
	void LoadCapabilities(const FInworldCapabilitySet& Capabilities);
	void LoadPlayerProfile(const FInworldPlayerProfile& PlayerProfile);
	
	EInworldConnectionState GetConnectionState() const;
	void GetConnectionError(FString& OutErrorMessage, int32& OutErrorCode) const;

	FString GetSessionId() const;

	TSharedPtr<FInworldPacket> SendTextMessage(const FString& AgentId, const FString& Text);
	TSharedPtr<FInworldPacket> SendTextMessageToConversation(const FString& ConversationId, const FString& Text);

	void SendSoundMessage(const FString& AgentId, class USoundWave* Sound);
	void SendSoundMessageToConversation(const FString& ConversationId, class USoundWave* Sound);
	void SendSoundDataMessage(const FString& AgentId, const TArray<uint8>& Data);
	void SendSoundDataMessageToConversation(const FString& ConversationId, const TArray<uint8>& Data);

	void SendSoundMessageWithEAC(const FString& AgentId, class USoundWave* Input, class USoundWave* Output);
	void SendSoundMessageWithEACToConversation(const FString& ConversationId, class USoundWave* Input, class USoundWave* Output);
	void SendSoundDataMessageWithEAC(const FString& AgentId, const TArray<uint8>& InputData, const TArray<uint8>& OutputData);
	void SendSoundDataMessageWithEACToConversation(const FString& ConversationId, const TArray<uint8>& InputData, const TArray<uint8>& OutputData);

	void StartAudioSession(const FString& AgentId);
	void StartAudioSessionInConversation(const FString& ConversationId);
	void StopAudioSession(const FString& AgentId);
	void StopAudioSessionInConversation(const FString& ConversationId);

	void SendCustomEvent(const FString& AgentId, const FString& Name, const TMap<FString, FString>& Params);
	void SendCustomEventToConversation(const FString& ConversationId, const FString& Name, const TMap<FString, FString>& Params);
	void SendChangeSceneEvent(const FString& SceneName);

	void SendNarrationEvent(const FString& AgentId, const FString& Content);

	void CancelResponse(const FString& AgentId, const FString& InteractionId, const TArray<FString>& UtteranceIds);

	FOnInworldSessionSaved OnSessionSaved;

	FOnInworldLatency OnPerceivedLatency;
	
	FOnInworldConnectionStateChanged OnConnectionStateChanged;

	FOnInworldPacketReceived OnInworldPacketReceived;

private:

	bool bIsBeingDestroyed = false;

#if !UE_BUILD_SHIPPING
	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnAudioDumperCVarChanged, bool /*Enabled*/, FString /*Path*/);
	static FOnAudioDumperCVarChanged OnAudioDumperCVarChanged;
	FDelegateHandle OnAudioDumperCVarChangedHandle;
	static FAutoConsoleVariableSink CVarSink;
	static void OnCVarsChanged();
#endif

};
