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

#if !UE_BUILD_SHIPPING
	// used in a function template from NDK
	#define INWORLD_AUDIO_DUMP 1
#endif

namespace Inworld
{
	class FClient;
}

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
	
	EInworldConnectionState GetConnectionState() const;
	void GetConnectionError(FString& OutErrorMessage, int32& OutErrorCode) const;

	FString GetSessionId() const;

	TSharedPtr<FInworldPacket> SendTextMessage(const FString& AgentId, const FString& Text);

	void SendSoundMessage(const FString& AgentId, class USoundWave* Sound);
	void SendSoundDataMessage(const FString& AgentId, const TArray<uint8>& Data);

	void SendSoundMessageWithEAC(const FString& AgentId, class USoundWave* Input, class USoundWave* Output);
	void SendSoundDataMessageWithEAC(const FString& AgentId, const TArray<uint8>& InputData, const TArray<uint8>& OutputData);

	void StartAudioSession(const FString& AgentId);
	void StopAudioSession(const FString& AgentId);

	void SendCustomEvent(const FString& AgentId, const FString& Name, const TMap<FString, FString>& Params);
	void SendChangeSceneEvent(const FString& SceneName);

	void CancelResponse(const FString& AgentId, const FString& InteractionId, const TArray<FString>& UtteranceIds);

	FOnInworldSceneLoaded OnSceneLoaded;

	FOnInworldSessionSaved OnSessionSaved;

	FOnInworldLatency OnPerceivedLatency;
	
	FOnInworldConnectionStateChanged OnConnectionStateChanged;

	FOnInworldPacketReceived OnInworldPacketReceived;

private:
	FString GenerateUserId();

	TSharedPtr<Inworld::FClient> InworldClient;

#if !UE_BUILD_SHIPPING
	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnAudioDumperCVarChanged, bool /*Enabled*/, FString /*Path*/);
	static FOnAudioDumperCVarChanged OnAudioDumperCVarChanged;
	FDelegateHandle OnAudioDumperCVarChangedHandle;
	static FAutoConsoleVariableSink CVarSink;
	static void OnCVarsChanged();
#endif

};
