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
#include "ThirdParty/InworldAINDKLibrary/include/AECInterop.h"

void UInworldAudioSender::Initialize(bool bEnableVAD)
{
	bVADEnabled = bEnableVAD;
	if (bVADEnabled)
	{
		Inworld::VAD_Initialize("model");
	}
	AecHandle = WebRtcAec3_Create(16000);
}

void UInworldAudioSender::Terminate()
{
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
	ClearState();
	WebRtcAec3_Free(AecHandle);
	if (bVADEnabled)
	{
		Inworld::VAD_Terminate();
	}
}

void UInworldAudioSender::ClearState()
{
	StopActualSession();
	RoutingId = {};
	AudioQueue = {};
	MicMode = EInworldMicrophoneMode::UNKNOWN;
	VADSilenceCounter = 0;
}

void UInworldAudioSender::StartAudioSession(const std::string& AgentId, EInworldMicrophoneMode MicrophoneMode)
{
	ClearState();
	RoutingId = AgentId;
	bConversation = false;
	MicMode = MicrophoneMode;
	if (!bVADEnabled)
	{
		StartActualSession();
	}
}

void UInworldAudioSender::StartAudioSessionInConversation(const std::string& ConversationId, EInworldMicrophoneMode MicrophoneMode)
{
	ClearState();
	RoutingId = ConversationId;
	bConversation = true;
	MicMode = MicrophoneMode;
	if (!bVADEnabled)
	{
		StartActualSession();
	}
}

void UInworldAudioSender::StopAudioSession(const std::string& AgentId)
{
	ClearState();
	if (!bVADEnabled)
	{
		StopActualSession();
	}
}

void UInworldAudioSender::StopAudioSessionInConversation(const std::string& ConversationId)
{
	ClearState();
	if (!bVADEnabled)
	{
		StopActualSession();
	}
}

void UInworldAudioSender::SendSoundMessage(const std::string& AgentId, const std::vector<int16_t>& InputData)
{
	if (bConversation || RoutingId != AgentId)
	{
		UE_LOG(LogInworldAIClient, Warning, TEXT("UInworldAudioSender::SendSoundMessage invalid routing: Agent %hs."), AgentId.c_str());
		return;
	}

	ProcessAudio(InputData, {});
}

void UInworldAudioSender::SendSoundMessageToConversation(const std::string& ConversationId, const std::vector<int16_t>& InputData)
{
	if (!bConversation || RoutingId != ConversationId)
	{
		UE_LOG(LogInworldAIClient, Warning, TEXT("UInworldAudioSender::SendSoundMessage invalid routing: Conversation %hs."), ConversationId.c_str());
		return;
	}

	ProcessAudio(InputData, {});
}

void UInworldAudioSender::SendSoundMessageWithAEC(const std::string& AgentId, const std::vector<int16_t>& InputData, const std::vector<int16_t>& OutputData)
{
	if (bConversation || RoutingId != AgentId)
	{
		UE_LOG(LogInworldAIClient, Warning, TEXT("UInworldAudioSender::SendSoundMessage invalid routing: Agent %hs."), AgentId.c_str());
		return;
	}

	ProcessAudio(InputData, OutputData);
}

void UInworldAudioSender::SendSoundMessageWithAECToConversation(const std::string& ConversationId, const std::vector<int16_t>& InputData, const std::vector<int16_t>& OutputData)
{
	if (!bConversation || RoutingId != ConversationId)
	{
		UE_LOG(LogInworldAIClient, Warning, TEXT("UInworldAudioSender::SendSoundMessage invalid routing: Conversation %hs."), ConversationId.c_str());
		return;
	}

	ProcessAudio(InputData, OutputData);
}

void UInworldAudioSender::StartActualSession()
{
	if (bSessionActive)
	{
		return;
	}

	Inworld::AudioSessionStartPayload AudioSessionStartPayload;
	AudioSessionStartPayload.MicMode = static_cast<Inworld::AudioSessionStartPayload::MicrophoneMode>(MicMode);
	if (bConversation)
	{
		Inworld::GetClient()->StartAudioSessionInConversation(RoutingId, AudioSessionStartPayload);
	}
	else
	{
		Inworld::GetClient()->StartAudioSession(RoutingId, AudioSessionStartPayload);
	}
	bSessionActive = true;
	UE_LOG(LogInworldAIClient, Log, TEXT("UInworldAudioSender start actual audio session."));
}

void UInworldAudioSender::StopActualSession()
{
	if (!bSessionActive)
	{
		return;
	}
	
	if (bConversation)
	{
		Inworld::GetClient()->StopAudioSessionInConversation(RoutingId);
	}
	else
	{
		Inworld::GetClient()->StopAudioSession(RoutingId);
	}
	bSessionActive = false;
	UE_LOG(LogInworldAIClient, Log, TEXT("UInworldAudioSender stop actual audio session."));
}

void UInworldAudioSender::ProcessAudio(const std::vector<int16_t>& InputData, const std::vector<int16_t>& OutputData)
{
	constexpr float VADProbThreshhold = 0.3f;
	constexpr int8_t VADPreviousChunks = 5;
	constexpr int8_t VADSubsequentChunks = 5;

	GEngine->AddOnScreenDebugMessage(111, 0.12f, FColor::Red, FString::Printf(TEXT("NOT SENDING AUDIO")));
	
	const std::vector<int16_t> FilteredData = OutputData.empty() ? InputData : ApplyAEC(InputData, OutputData);
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

	const bool bSpeech = Inworld::VAD_Process(FloatData.data(), FloatData.size()) > VADProbThreshhold;
	if (bSpeech)
	{
		StartActualSession();
		AudioQueue.push(Data);
		AdvanceAudioQueue();
		return;
	}

	if (!bSessionActive)
	{
		AudioQueue.push(Data);
		if (AudioQueue.size() > VADPreviousChunks)
		{
			AudioQueue.pop();
		}
		return;
	}

	SendAudio(Data);
	if (++VADSilenceCounter > VADSubsequentChunks)
	{
		StopActualSession();
		VADSilenceCounter = 0;
	}
}

std::vector<int16_t> UInworldAudioSender::ApplyAEC(const std::vector<int16_t>& InputData, const std::vector<int16_t>& OutputData)
{
	std::vector<int16_t> FilteredAudio = InputData;
	constexpr int32_t NumSamples = 160;
	const int32_t MaxSamples = std::min(InputData.size(), OutputData.size()) / NumSamples * NumSamples;

	for (int32_t i = 0; i < MaxSamples; i += NumSamples)
	{
		WebRtcAec3_BufferFarend(AecHandle, OutputData.data() + i);
		WebRtcAec3_Process(AecHandle, InputData.data() + i, FilteredAudio.data() + i);
	}
	return FilteredAudio;
}

void UInworldAudioSender::SendAudio(const std::string& Data)
{
	if (RoutingId.empty())
	{
		UE_LOG(LogInworldAIClient, Warning, TEXT("UInworldAudioSender::SendAudio invalid routing."));
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

	GEngine->AddOnScreenDebugMessage(111, 0.12f, FColor::Green, FString::Printf(TEXT("SENDING AUDIO")));
}

void UInworldAudioSender::AdvanceAudioQueue()
{
	SendAudio(AudioQueue.front());
	AudioQueue.pop();

	if (!AudioQueue.empty())
	{
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
		{
			AdvanceAudioQueue();
		}, 10, false);
	}
}
