/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include <queue>
#include <string>
#include <vector>

#include "CoreMinimal.h"
#include "InworldEnums.h"

#include "InworldAudioSender.generated.h"

class UInworldPlayer;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnInworldVADNative, UInworldPlayer*, bool);

UCLASS()
class INWORLDAICLIENT_API UInworldAudioSender : public UObject
{
public:
	GENERATED_BODY()
	
	void Initialize(bool bEnableVAD);
	void Terminate();

	void StartAudioSession(const std::string& AgentId, UInworldPlayer* Player, EInworldMicrophoneMode MicMode);
	void StartAudioSessionInConversation(const std::string& ConversationId, UInworldPlayer* Player, EInworldMicrophoneMode MicMode);
		
	void StopAudioSession(const std::string& AgentId);
	void StopAudioSessionInConversation(const std::string& ConversationId);

	void SendSoundMessage(const std::string& AgentId, const std::vector<int16_t>& InputData);
	void SendSoundMessageToConversation(const std::string& ConversationId, const std::vector<int16_t>& InputData);
		
	void SendSoundMessageWithAEC(const std::string& AgentId, const std::vector<int16_t>& InputData, const std::vector<int16_t>& OutputData);
	void SendSoundMessageWithAECToConversation(const std::string& ConversationId, const std::vector<int16_t>& InputData, const std::vector<int16_t>& OutputData);
	
	FOnInworldVADNative& OnVAD() { return OnVADNative; }
	
private:
	void StartActualAudioSession();
	void StopActualAudioSession();
	void ProcessAudio(const std::vector<int16_t>& InputData, const std::vector<int16_t>& OutputData);
	std::vector<int16_t>  ApplyAEC(const std::vector<int16_t>& InputData, const std::vector<int16_t>& OutputData);
	void SendAudio(const std::string& Data);
	void AdvanceAudioQueue();
	void ClearState();

	FOnInworldVADNative OnVADNative;

	UInworldPlayer* PlayerSender = nullptr;
	FTimerHandle TimerHandle;
	std::queue<std::string> AudioQueue;
	void* AecHandle = nullptr;
	EInworldMicrophoneMode MicMode = EInworldMicrophoneMode::UNKNOWN;
	std::string RoutingId;
	bool bVADEnabled = false;
	bool bConversation = false;
	bool bSessionActive = false;
	int8_t VADSilenceCounter = 0;
};
