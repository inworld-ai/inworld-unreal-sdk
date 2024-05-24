/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "CoreMinimal.h"
#include "InworldEnums.h"

namespace Inworld
{
	class AECFilter;
}

class FInworldAudioSender
{
public:
	void Initialize();
	void Terminate();
	void EnableVAD(bool bVal);

	void StartAudioSession(const std::string& AgentId, EInworldMicrophoneMode MicMode);
	void StartAudioSessionInConversation(const std::string& ConversationId, EInworldMicrophoneMode MicMode);
		
	void StopAudioSession(const std::string& AgentId);
	void StopAudioSessionInConversation(const std::string& ConversationId);

	void SendSoundMessage(const std::string& AgentId, const std::vector<int16_t>& InputData);
	void SendSoundMessageToConversation(const std::string& ConversationId, const std::vector<int16_t>& InputData);
		
	void SendSoundMessageWithAEC(const std::string& AgentId, const std::vector<int16_t>& InputData, const std::vector<int16_t>& OutputData);
	void SendSoundMessageWithAECToConversation(const std::string& ConversationId, const std::vector<int16_t>& InputData, const std::vector<int16_t>& OutputData);

private:
	void StartActualSession();
	void StopActualSession();
	void ProcessAudio(const std::vector<int16_t>& InputData, const std::vector<int16_t>& OutputData);
	void SendAudio(const std::string& Data);
	void ClearState();
	
	Inworld::AECFilter* AECFilter = nullptr;
	EInworldMicrophoneMode MicMode = EInworldMicrophoneMode::UNKNOWN;
	std::string RoutingId;
	bool bVADEnabled = false;
	bool bConversation = false;
	bool bSessionActive = false;
	int8_t VADSilenceCounter = 0;
};
