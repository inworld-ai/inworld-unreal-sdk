/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldAudioSender.h"

#include "Client.h"
#include "InworldAIClientModule.h"
#ifdef INWORLD_VAD
#include "ThirdParty/InworldAINDKLibrary/include/InworldVAD.h"
#endif
#ifdef INWORLD_AEC
#include "ThirdParty/InworldAINDKLibrary/include/AECInterop.h"
#endif

void UInworldAudioSender::Initialize(bool bEnableVAD)
{
	bVADEnabled = bEnableVAD;
	if (bVADEnabled)
	{
#ifdef INWORLD_VAD
	Inworld::VAD_Initialize("model");
#endif
	}
#ifdef INWORLD_AEC
	AecHandle = WebRtcAec3_Create(16000);
#endif
}

void UInworldAudioSender::Terminate()
{
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
	ClearState();
#ifdef INWORLD_AEC
	WebRtcAec3_Free(AecHandle);
#endif
	if (bVADEnabled)
	{
#ifdef INWORLD_VAD
	Inworld::VAD_Terminate();
#endif
	}
}

void UInworldAudioSender::ClearState()
{
	StopActualAudioSession();
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
		StartActualAudioSession();
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
		StartActualAudioSession();
	}
}

void UInworldAudioSender::StopAudioSession(const std::string& AgentId)
{
	ClearState();
	if (!bVADEnabled)
	{
		StopActualAudioSession();
	}
}

void UInworldAudioSender::StopAudioSessionInConversation(const std::string& ConversationId)
{
	ClearState();
	if (!bVADEnabled)
	{
		StopActualAudioSession();
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

void UInworldAudioSender::StartActualAudioSession()
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

void UInworldAudioSender::StopActualAudioSession()
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

	const bool bVad =
#ifdef INWORLD_VAD
		bVADEnabled;
#else
		false;
#endif
	if (!bVad)
	{
		SendAudio(Data);
		return;
	}
	
	std::vector<float> FloatData(FilteredData.size());
	for (size_t i = 0; i < FilteredData.size(); ++i)
	{
		FloatData[i] = static_cast<float>(FilteredData[i]) / 32767.0f;
	}

	const float SpeechProb =
#ifdef INWORLD_VAD
		Inworld::VAD_Process(FloatData.data(), FloatData.size());
#else
		1.f;
#endif

	GEngine->AddOnScreenDebugMessage(112, 0.12f, SpeechProb > VADProbThreshhold ? FColor::Green : FColor::Red,
		FString::Printf(TEXT("Speech prob: %f"), SpeechProb));
	
	if (SpeechProb > VADProbThreshhold)
	{
		StartActualAudioSession();
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
		StopActualAudioSession();
		VADSilenceCounter = 0;
	}
}

std::vector<int16_t> UInworldAudioSender::ApplyAEC(const std::vector<int16_t>& InputData, const std::vector<int16_t>& OutputData)
{
#ifdef INWORLD_AEC
	std::vector<int16_t> FilteredAudio = InputData;
	constexpr int32_t NumSamples = 160;
	const int32_t MaxSamples = std::min(InputData.size(), OutputData.size()) / NumSamples * NumSamples;

	for (int32_t i = 0; i < MaxSamples; i += NumSamples)
	{
		WebRtcAec3_BufferFarend(AecHandle, OutputData.data() + i);
		WebRtcAec3_Process(AecHandle, InputData.data() + i, FilteredAudio.data() + i);
	}
	return FilteredAudio;
#else
	return InputData;
#endif
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
	// unwind the queue sending audio every 5ms
	// data loss if send all at once
	
	SendAudio(AudioQueue.front());
	AudioQueue.pop();

	if (!AudioQueue.empty())
	{
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
		{
			AdvanceAudioQueue();
		}, 0.005f, false);
	}
}
