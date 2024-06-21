/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldAudioSender.h"

#include "Client.h"
#include "InworldAIClientModule.h"
#include "Interfaces/IPluginManager.h"
#include "Engine/Engine.h"
#ifdef INWORLD_VAD
#include "ThirdParty/InworldAINDKLibrary/include/InworldVAD.h"
#endif
#ifdef INWORLD_AEC
#include "ThirdParty/InworldAINDKLibrary/include/AECInterop.h"
#endif

static TAutoConsoleVariable<bool> CVarDisableVAD(
	TEXT("Inworld.Debug.VADForceDisable"), false,
	TEXT("Force disable VAD")
);
static TAutoConsoleVariable<bool> CVarShowSendAudioMessage(
	TEXT("Inworld.Debug.VADShowSendAudioMessage"), false,
	TEXT("Force disable VAD")
	);
static TAutoConsoleVariable<float> CVarBufferedAudioSendDelay(
	TEXT("Inworld.Debug.VADBufferedAudioSendDelay"), 0.03f,
	TEXT("Force disable VAD")
);

void UInworldAudioSender::Initialize(bool bEnableVAD)
{
#ifdef INWORLD_VAD
	bVADEnabled = bEnableVAD;
	if (CVarDisableVAD->GetBool())
	{
		bVADEnabled = false;
	}
	if (bVADEnabled)
	{
		const FString Path = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("InworldAI"))->GetBaseDir(), TEXT("Source/ThirdParty/InworldAINDKLibrary/resource/silero_vad_10_27_2022.onnx"));
		const std::string ModelPath = TCHAR_TO_UTF8(*Path);
		Inworld::VAD_Initialize(ModelPath.c_str());
	}
#else
	bVADEnabled = false;
#endif
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
#ifdef INWORLD_VAD
	if (bVADEnabled)
	{
		Inworld::VAD_Terminate();
	}
#endif
}

void UInworldAudioSender::ClearState()
{
	StopActualAudioSession();
	RoutingId = {};
	AudioQueue = {};
	MicMode = EInworldMicrophoneMode::UNKNOWN;
	VADSilenceCounter = 0;
	SessionOwner = nullptr;
#ifdef INWORLD_VAD
	if (bVADEnabled)
	{
		Inworld::VAD_ResetState();
	}
#endif
}

void UInworldAudioSender::StartAudioSession(const std::string& AgentId, UObject* Owner, EInworldMicrophoneMode MicrophoneMode)
{
	ClearState();
	RoutingId = AgentId;
	bConversation = false;
	MicMode = MicrophoneMode;
	SessionOwner = Owner;
	if (!bVADEnabled)
	{
		StartActualAudioSession();
	}
}

void UInworldAudioSender::StartAudioSessionInConversation(const std::string& ConversationId, UObject* Owner, EInworldMicrophoneMode MicrophoneMode)
{
	ClearState();
	RoutingId = ConversationId;
	bConversation = true;
	MicMode = MicrophoneMode;
	SessionOwner = Owner;
	if (!bVADEnabled)
	{
		StartActualAudioSession();
	}
}

void UInworldAudioSender::StopAudioSession(const std::string& AgentId)
{
	if (!bVADEnabled)
	{
		StopActualAudioSession();
	}
	ClearState();
}

void UInworldAudioSender::StopAudioSessionInConversation(const std::string& ConversationId)
{
	if (!bVADEnabled)
	{
		StopActualAudioSession();
	}
	ClearState();
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

bool UInworldAudioSender::StartActualAudioSession()
{
	if (bSessionActive)
	{
		return false;
	}

	Inworld::AudioSessionStartPayload AudioSessionStartPayload;
	const EInworldMicrophoneMode Mode = /*bVADEnabled ? EInworldMicrophoneMode::EXPECT_AUDIO_END :*/ MicMode;
	AudioSessionStartPayload.MicMode = static_cast<Inworld::AudioSessionStartPayload::MicrophoneMode>(Mode);
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
	if (bVADEnabled)
	{
		OnVADNative.Broadcast(SessionOwner, true);
	}
	return true;
}

bool UInworldAudioSender::StopActualAudioSession()
{
	if (!bSessionActive)
	{
		return false;
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
	if (bVADEnabled)
	{
		OnVADNative.Broadcast(SessionOwner, false);
	}
	return true;
}

void UInworldAudioSender::ProcessAudio(const std::vector<int16_t>& InputData, const std::vector<int16_t>& OutputData)
{
	constexpr float VADProbThreshhold = 0.3f;
	constexpr int8_t VADPreviousChunks = 5;
	constexpr int8_t VADSubsequentChunks = 5;

	if (CVarShowSendAudioMessage->GetBool())
	{
		GEngine->AddOnScreenDebugMessage(111, 0.12f, FColor::Red, FString::Printf(TEXT("NOT SENDING AUDIO")));
	}
	
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

	const float SpeechProb =
#ifdef INWORLD_VAD
		Inworld::VAD_Process(FloatData.data(), FloatData.size());
#else
		1.f;
#endif
	
	if (SpeechProb > VADProbThreshhold)
	{
		VADSilenceCounter = 0;
		const bool bJustStarted = StartActualAudioSession();
		if (bJustStarted)
		{
			AudioQueue.push(Data);
			SendBufferedAudio();
		}
		else
		{
			SendAudio(Data);
		}
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

	if (CVarShowSendAudioMessage->GetBool())
	{
		GEngine->AddOnScreenDebugMessage(111, 0.12f, FColor::Green, FString::Printf(TEXT("SENDING AUDIO")));
	}
}

void UInworldAudioSender::SendBufferedAudio()
{
	if (AudioQueue.empty())
	{
		return;
	}
	
	std::string Data;
	while (!AudioQueue.empty())
	{
		Data.append(AudioQueue.front());
		AudioQueue.pop();
	}

	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, Data]()
	{
		SendAudio(Data);
	}, CVarBufferedAudioSendDelay->GetFloat(), false);
}
