/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldAudioSender.h"

#include "Client.h"
#include "InworldAIClientModule.h"
#include "ThirdParty/InworldAINDKLibrary/include/InworldVAD.h"

void FInworldAudioSender::Initialize()
{
	AECFilter = new Inworld::AECFilter();
	Inworld::VAD_Initialize("model");
}

void FInworldAudioSender::Terminate()
{
	delete AECFilter;
	Inworld::VAD_Terminate();
}

void FInworldAudioSender::EnableVAD(bool bVal)
{
	bVADEnabled = bVal;
}

void FInworldAudioSender::ClearState()
{
	StopActualSession();
	RoutingId = {};
	MicMode = EInworldMicrophoneMode::UNKNOWN;
	VADSilenceCounter = 0;
}

void FInworldAudioSender::StartAudioSession(const std::string& AgentId, EInworldMicrophoneMode MicrophoneMode)
{
	ClearState();
	RoutingId = AgentId;
	bConversation = false;
	MicMode = MicrophoneMode;
}

void FInworldAudioSender::StartAudioSessionInConversation(const std::string& ConversationId, EInworldMicrophoneMode MicrophoneMode)
{
	ClearState();
	RoutingId = ConversationId;
	bConversation = true;
	MicMode = MicrophoneMode;
}

void FInworldAudioSender::StopAudioSession(const std::string& AgentId)
{
	ClearState();
}

void FInworldAudioSender::StopAudioSessionInConversation(const std::string& ConversationId)
{
	ClearState();
}

void FInworldAudioSender::SendSoundMessage(const std::string& AgentId, const std::vector<int16_t>& InputData)
{
	if (bConversation || RoutingId != AgentId)
	{
		UE_LOG(LogInworldAIClient, Warning, TEXT("FInworldAudioSender::SendSoundMessage invalid routing: Agent %hs."), AgentId.c_str());
		return;
	}

	
}

void FInworldAudioSender::SendSoundMessageToConversation(const std::string& ConversationId, const std::vector<int16_t>& InputData)
{
	if (!bConversation || RoutingId != ConversationId)
	{
		UE_LOG(LogInworldAIClient, Warning, TEXT("FInworldAudioSender::SendSoundMessage invalid routing: Conversation %hs."), ConversationId.c_str());
		return;
	}
}

void FInworldAudioSender::SendSoundMessageWithAEC(const std::string& AgentId, const std::vector<int16_t>& InputData, const std::vector<int16_t>& OutputData)
{
	if (bConversation || RoutingId != AgentId)
	{
		UE_LOG(LogInworldAIClient, Warning, TEXT("FInworldAudioSender::SendSoundMessage invalid routing: Agent %hs."), AgentId.c_str());
		return;
	}
}

void FInworldAudioSender::SendSoundMessageWithAECToConversation(const std::string& ConversationId, const std::vector<int16_t>& InputData, const std::vector<int16_t>& OutputData)
{
	if (bConversation || RoutingId != ConversationId)
	{
		UE_LOG(LogInworldAIClient, Warning, TEXT("FInworldAudioSender::SendSoundMessage invalid routing: Conversation %hs."), ConversationId.c_str());
		return;
	}
}

void FInworldAudioSender::StartActualSession()
{
	if (!bSessionActive)
	{
		Inworld::AudioSessionStartPayload AudioSessionStartPayload;
		AudioSessionStartPayload.MicMode = static_cast<Inworld::AudioSessionStartPayload::MicrophoneMode>(MicMode);
		Inworld::GetClient()->StartAudioSession(RoutingId, AudioSessionStartPayload);
		bSessionActive = true;
		UE_LOG(LogInworldAIClient, Warning, TEXT("FInworldAudioSender::ProcessAudio start actual audio session."));
		GEngine->AddOnScreenDebugMessage(111, 0.12f, FColor::Green, FString::Printf(TEXT("Start audio session.")));
	}
}

void FInworldAudioSender::StopActualSession()
{
	if (bSessionActive)
	{
		Inworld::GetClient()->StopAudioSession(RoutingId);
		bSessionActive = false;
		UE_LOG(LogInworldAIClient, Warning, TEXT("FInworldAudioSender::ClearState stop actual audio session."));
		GEngine->AddOnScreenDebugMessage(112, 0.12f, FColor::Red, FString::Printf(TEXT("Stop audio session.")));
	}
}

void FInworldAudioSender::ProcessAudio(const std::vector<int16_t>& InputData, const std::vector<int16_t>& OutputData)
{
	constexpr float VADThreshhold = 0.3f;
	constexpr int8_t VADMaxSilence = 10;
	
	const std::vector<int16_t> FilteredData = OutputData.empty() ? InputData : AECFilter->FilterAudio(InputData, OutputData);
	std::string Data((char*)FilteredData.data(), FilteredData.size() * 2);
	if (!bVADEnabled)
	{
		SendAudio(Data);
		return;
	}
	
	std::vector<float> FloatData(FilteredData.size());
	for (size_t i = 0; i < FilteredData.size(); ++i)
	{
		FloatData[i] = static_cast<float>(FilteredData[i]) / 32767.0f;
	}

	const bool bSpeech = Inworld::VAD_Process(FloatData.data(), FloatData.size()) > VADThreshhold;
	if (bSpeech)
	{
		StartActualSession();
		SendAudio(Data);
		return;
	}

	if (!bSessionActive)
	{
		return;
	}

	SendAudio(Data);
	if (++VADSilenceCounter > VADMaxSilence)
	{
		StopActualSession();
		VADSilenceCounter = 0;
	}
}

void FInworldAudioSender::SendAudio(const std::string& Data)
{
	if (RoutingId.empty())
	{
		UE_LOG(LogInworldAIClient, Warning, TEXT("FInworldAudioSender::SendAudio invalid routing."));
		return;
	}
	
	if (bConversation)
	{
		Inworld::GetClient()->SendSoundMessageToConversation(RoutingId, Data);
	}
	else
	{
		Inworld::GetClient()->SendSoundMessage(RoutingId, Data);
	}
}
