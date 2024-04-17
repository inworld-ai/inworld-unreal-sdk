/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldClient.h"
#include "InworldAIClientModule.h"
#include "CoreMinimal.h"
#include "GenericPlatform/GenericPlatformMisc.h"

#include "SocketSubsystem.h"
#include "IPAddress.h"

#include "InworldUtils.h"
#include "InworldPacketTranslator.h"
#include "Editor/Experimental/EditorInteractiveToolsFramework/Public/Behaviors/2DViewportBehaviorTargets.h"
#include "Editor/Experimental/EditorInteractiveToolsFramework/Public/Behaviors/2DViewportBehaviorTargets.h"

THIRD_PARTY_INCLUDES_START
#include "Packets.h"
#include "Client.h"
#include "Utils/Log.h"
THIRD_PARTY_INCLUDES_END

#include "Async/Async.h"
#include "Async/TaskGraphInterfaces.h"

#include <Interfaces/IPluginManager.h>

#include "Misc/Paths.h"

const FString DefaultTargetUrl = "api-engine.inworld.ai:443";

#include <string>

#if !UE_BUILD_SHIPPING

static TAutoConsoleVariable<bool> CVarEnableSoundDump(
	TEXT("Inworld.Debug.EnableSoundDump"), false,
	TEXT("Enable/Disable recording audio input to dump file")
);

static TAutoConsoleVariable<FString> CVarSoundDumpPath(
	TEXT("Inworld.Debug.SoundDumpPath"),
#if WITH_EDITOR
	FPaths::ConvertRelativePathToFull(FPaths::ProjectLogDir().Append("InworldAudioDump.wav")),
#else
	FPaths::ConvertRelativePathToFull(FPaths::ProjectDir().Append("InworldAudioDump.wav")),
#endif // WITH_EDITOR
	TEXT("Specifiy path for audio input dump file")
);

FInworldClient::FOnAudioDumperCVarChanged FInworldClient::OnAudioDumperCVarChanged;

FAutoConsoleVariableSink FInworldClient::CVarSink(FConsoleCommandDelegate::CreateStatic(&FInworldClient::OnCVarsChanged));
#endif

void FInworldClient::Init()
{
	FString ClientVer;
	TSharedPtr<IPlugin> InworldAIPlugin = IPluginManager::Get().FindPlugin("InworldAI");
	if (ensure(InworldAIPlugin.IsValid()))
	{
		ClientVer = InworldAIPlugin.Get()->GetDescriptor().VersionName;
	}
	FString ClientId("unreal");

	Inworld::SdkInfo Sdk;
	Sdk.Type = TCHAR_TO_UTF8(*ClientId);
	Sdk.Version = TCHAR_TO_UTF8(*ClientVer);
	
	FString OSVersion, OSSubversion;
	FPlatformMisc::GetOSVersions(OSVersion, OSSubversion);
	FString OSFullVersion = FString::Printf(TEXT("%s %s"), *OSVersion, *OSSubversion);
	Sdk.OS = TCHAR_TO_UTF8(*OSFullVersion);

	Inworld::CreateClient();
	Inworld::GetClient()->InitClientAsync(Sdk,
		[this](Inworld::Client::ConnectionState ConnectionState)
		{
			if (bIsBeingDestroyed)
			{
				return;
			}

			AsyncTask(ENamedThreads::GameThread, [this, ConnectionState]() 
			{
				OnConnectionStateChanged.ExecuteIfBound(static_cast<EInworldConnectionState>(ConnectionState));
			});
		},
		[this](std::shared_ptr<Inworld::Packet> Packet)
		{
			AsyncTask(ENamedThreads::GameThread, [this, Packet]()
			{
				InworldPacketTranslator PacketTranslator;
				Packet->Accept(PacketTranslator);
				TSharedPtr<FInworldPacket> ReceivedPacket = PacketTranslator.GetPacket();
				if (ReceivedPacket.IsValid())
				{
					OnInworldPacketReceived.ExecuteIfBound(ReceivedPacket);
				}
			});
		}
	);

	Inworld::GetClient()->SetPerceivedLatencyTrackerCallback([this](const std::string& InteractionId, uint32_t LatancyMs)
		{
			OnPerceivedLatency.ExecuteIfBound(UTF8_TO_TCHAR(InteractionId.c_str()), LatancyMs);
		}
	);

#if !UE_BUILD_SHIPPING
	auto OnAudioDumperCVarChangedCallback = [this](bool bEnable, FString Path)
	{
		const auto DumpPath = CVarSoundDumpPath.GetValueOnGameThread();
		if (!FPaths::DirectoryExists(FPaths::GetPath(DumpPath)))
		{
			UE_LOG(LogInworldAIClient, Error, TEXT("Audio dump path is invalid: %s."), *DumpPath);
			Inworld::GetClient()->SetAudioDumpEnabled(false, "");
			return;
		}
		const std::string StdPath = TCHAR_TO_UTF8(*DumpPath);
		Inworld::GetClient()->SetAudioDumpEnabled(false, StdPath);

		if (bEnable)
		{
			UE_LOG(LogInworldAIClient, Log, TEXT("Audio dump path: %s."), *DumpPath);
			Inworld::GetClient()->SetAudioDumpEnabled(true, StdPath);
		}
	};
	OnAudioDumperCVarChangedHandle = OnAudioDumperCVarChanged.AddLambda(OnAudioDumperCVarChangedCallback);
	OnAudioDumperCVarChangedCallback(CVarEnableSoundDump.GetValueOnGameThread(), CVarSoundDumpPath.GetValueOnGameThread());
#endif
}

void FInworldClient::Destroy()
{
	bIsBeingDestroyed = true;
#if !UE_BUILD_SHIPPING
	OnAudioDumperCVarChanged.Remove(OnAudioDumperCVarChangedHandle);
#endif
	Inworld::DestroyClient();
}

static void ConvertCapabilities(const FInworldCapabilitySet& Capabilities, Inworld::Capabilities& OutCapabilities)
{
	OutCapabilities.Animations = Capabilities.Animations;
	OutCapabilities.Audio = Capabilities.Audio;
	OutCapabilities.Emotions = Capabilities.Emotions;
	OutCapabilities.Interruptions = Capabilities.Interruptions;
	OutCapabilities.EmotionStreaming = Capabilities.EmotionStreaming;
	OutCapabilities.SilenceEvents = Capabilities.SilenceEvents;
	OutCapabilities.PhonemeInfo = Capabilities.PhonemeInfo;
	OutCapabilities.Continuation = Capabilities.Continuation;
	OutCapabilities.TurnBasedSTT = Capabilities.TurnBasedSTT;
	OutCapabilities.NarratedActions = Capabilities.NarratedActions;
	OutCapabilities.Relations = Capabilities.Relations;
	OutCapabilities.Multiagent = Capabilities.MultiAgent;
}

static FString GenerateUserId()
{
	FString Id = FPlatformMisc::GetDeviceId();
	if (Id.IsEmpty())
	{
#if PLATFORM_WINDOWS || PLATFORM_MAC
		Id = FPlatformMisc::GetMacAddressString();
#endif
	}
	if (Id.IsEmpty())
	{
		UE_LOG(LogInworldAIClient, Error, TEXT("Couldn't generate user id."));
		return FString();
	}

	UE_LOG(LogInworldAIClient, Log, TEXT("Device Id: %s"), *Id);

	std::string SId = TCHAR_TO_UTF8(*Id);
	TArray<uint8> Data;
	Data.SetNumZeroed(SId.size());
	FMemory::Memcpy(Data.GetData(), SId.data(), SId.size());

	Data = Inworld::Utils::HmacSha256(Data, Data);

	return FString(UTF8_TO_TCHAR(Inworld::Utils::ToHex(Data).c_str()));
}

static void ConvertPlayerProfile(const FInworldPlayerProfile& PlayerProfile, Inworld::UserConfiguration& UserConfig)
{
	UserConfig.Name = TCHAR_TO_UTF8(*PlayerProfile.Name);
	UserConfig.Id = PlayerProfile.UniqueId.IsEmpty() ? TCHAR_TO_UTF8(*GenerateUserId()) : TCHAR_TO_UTF8(*PlayerProfile.UniqueId);
	UserConfig.Profile.Fields.reserve(PlayerProfile.Fields.Num());
	for (const auto& ProfileField : PlayerProfile.Fields)
	{
		Inworld::UserConfiguration::PlayerProfile::PlayerField PlayerField;
		PlayerField.Id = TCHAR_TO_UTF8(*ProfileField.Key);
		PlayerField.Value = TCHAR_TO_UTF8(*ProfileField.Value);
		UserConfig.Profile.Fields.push_back(PlayerField);
	}
}

void FInworldClient::Start(const FString& SceneName, const FInworldPlayerProfile& PlayerProfile, const FInworldCapabilitySet& Capabilities, const FInworldAuth& Auth, const FInworldSessionToken& SessionToken, const FInworldSave& Save, const FInworldEnvironment& Environment)
{
	Inworld::ClientOptions Options;
	Options.ServerUrl = TCHAR_TO_UTF8(*(!Environment.TargetUrl.IsEmpty() ? Environment.TargetUrl : DefaultTargetUrl));
	// Use first segment of scene for resource
	// 'workspaces/sample-workspace'
	TArray<FString> Split;
	SceneName.ParseIntoArray(Split, TEXT("/"));
	if (Split.Num() >= 2)
	{
		Options.Resource = TCHAR_TO_UTF8(*FString(Split[0] + "/" + Split[1]));
	}

	Options.SceneName = TCHAR_TO_UTF8(*SceneName);
	Options.Base64 = TCHAR_TO_UTF8(*Auth.Base64Signature);
	Options.ApiKey = TCHAR_TO_UTF8(*Auth.ApiKey);
	Options.ApiSecret = TCHAR_TO_UTF8(*Auth.ApiSecret);
	Options.ProjectName = TCHAR_TO_UTF8(*PlayerProfile.ProjectName);

	ConvertPlayerProfile(PlayerProfile, Options.UserConfig);
	ConvertCapabilities(Capabilities, Options.Capabilities);

	Inworld::SessionInfo Info;
	Info.Token = TCHAR_TO_UTF8(*SessionToken.Token);
	Info.ExpirationTime = SessionToken.ExpirationTime;
	Info.SessionId = TCHAR_TO_UTF8(*SessionToken.SessionId);

	if (Save.Data.Num() != 0)
    {
        Info.SessionSavedState.resize(Save.Data.Num());
        FMemory::Memcpy((uint8*)Info.SessionSavedState.data(), (uint8*)Save.Data.GetData(), Info.SessionSavedState.size());
    }

	Inworld::GetClient()->StartClient(Options, Info);
}

void FInworldClient::Stop()
{
	Inworld::GetClient()->StopClient();
}

void FInworldClient::Pause()
{
	Inworld::GetClient()->PauseClient();
}

void FInworldClient::Resume()
{
	Inworld::GetClient()->ResumeClient();
}

void FInworldClient::SaveSession()
{
	Inworld::GetClient()->SaveSessionStateAsync([this](std::string Data, bool bSuccess)
		{
			AsyncTask(ENamedThreads::GameThread, [this, Data, bSuccess]() {
				FInworldSave Save;
				if (!bSuccess)
				{
					UE_LOG(LogInworldAIClient, Error, TEXT("Couldn't generate user id."));
					OnSessionSaved.ExecuteIfBound(Save, false);
					return;
				}

				Save.Data.SetNumUninitialized(Data.size());
				FMemory::Memcpy((uint8*)Save.Data.GetData(), (uint8*)Data.data(), Save.Data.Num());

				OnSessionSaved.ExecuteIfBound(Save, true);
			});
		});
}

std::vector<std::string> ToStd(const TArray<FString>& Array)
{
	std::vector<std::string> Vec;
	for (auto& Str : Array)
	{
		Vec.emplace_back(TCHAR_TO_UTF8(*Str));
	}
	return Vec;
}

void FInworldClient::LoadCharacters(const TArray<FString>& Names)
{
	Inworld::GetClient()->LoadCharacters(ToStd(Names));
}

void FInworldClient::UnloadCharacters(const TArray<FString>& Names)
{
	Inworld::GetClient()->UnloadCharacters(ToStd(Names));
}

void FInworldClient::LoadSavedState(const TArray<uint8>& SavedState)
{
	std::string Data((char*)SavedState.GetData(), SavedState.Num());
	Inworld::GetClient()->LoadSavedState(Data);
}

void FInworldClient::LoadCapabilities(const FInworldCapabilitySet& Capabilities)
{
	Inworld::Capabilities Cpb;
	ConvertCapabilities(Capabilities, Cpb);
	Inworld::GetClient()->LoadCapabilities(Cpb);
}

void FInworldClient::LoadPlayerProfile(const FInworldPlayerProfile& PlayerProfile)
{
	Inworld::UserConfiguration UserConfig;
	ConvertPlayerProfile(PlayerProfile, UserConfig);
	Inworld::GetClient()->LoadUserConfiguration(UserConfig);
}

EInworldConnectionState FInworldClient::GetConnectionState() const
{
	return Inworld::GetClient() ? static_cast<EInworldConnectionState>(Inworld::GetClient()->GetConnectionState()) : EInworldConnectionState::Disconnected;
}

void FInworldClient::GetConnectionError(FString& OutErrorMessage, int32& OutErrorCode) const
{
	std::string OutError;
	Inworld::GetClient()->GetConnectionError(OutError, OutErrorCode);
	OutErrorMessage = UTF8_TO_TCHAR(OutError.c_str());
}

FString FInworldClient::GetSessionId() const
{
	return UTF8_TO_TCHAR(Inworld::GetSessionId().c_str());
}

FString FInworldClient::UpdateConversation(const FString& ConversationId, bool bIncludePlayer,
	const TArray<FString>& AgentIds)
{
	auto Packet = Inworld::GetClient()->UpdateConversation(ToStd(AgentIds), TCHAR_TO_UTF8(*ConversationId), bIncludePlayer);
	return UTF8_TO_TCHAR(Packet->_Routing._ConversationId.c_str());
}

TSharedPtr<FInworldPacket> FInworldClient::SendTextMessage(const FString& AgentId, const FString& Text)
{
	const auto Packet = Inworld::GetClient()->SendTextMessage(TCHAR_TO_UTF8(*AgentId), TCHAR_TO_UTF8(*Text));
	InworldPacketTranslator PacketTranslator;
	Packet->Accept(PacketTranslator);
	return PacketTranslator.GetPacket();
}

TSharedPtr<FInworldPacket> FInworldClient::SendTextMessageToConversation(const FString& ConversationId,	const FString& Text)
{
	const auto Packet = Inworld::GetClient()->SendTextMessageToConversation(TCHAR_TO_UTF8(*ConversationId), TCHAR_TO_UTF8(*Text));
	InworldPacketTranslator PacketTranslator;
	Packet->Accept(PacketTranslator);
	return PacketTranslator.GetPacket();
}

void FInworldClient::SendCustomEventToConversation(const FString& ConversationId, const FString& Name,
	const TMap<FString, FString>& Params)
{
}

void FInworldClient::SendSoundMessage(const FString& AgentId, class USoundWave* Sound)
{
	std::string data;
	if (Inworld::Utils::SoundWaveToString(Sound, data))
	{
		Inworld::GetClient()->SendSoundMessage(TCHAR_TO_UTF8(*AgentId), data);
	}
}

void FInworldClient::SendSoundMessageToConversation(const FString& ConversationId, USoundWave* Sound)
{
	std::string data;
	if (Inworld::Utils::SoundWaveToString(Sound, data))
	{
		Inworld::GetClient()->SendSoundMessageToConversation(TCHAR_TO_UTF8(*ConversationId), data);
	}
}

void FInworldClient::SendSoundDataMessage(const FString& AgentId, const TArray<uint8>& Data)
{
	std::string data((char*)Data.GetData(), Data.Num());
	Inworld::GetClient()->SendSoundMessage(TCHAR_TO_UTF8(*AgentId), data);
}

void FInworldClient::SendSoundDataMessageToConversation(const FString& ConversationId, const TArray<uint8>& Data)
{
	std::string data((char*)Data.GetData(), Data.Num());
	Inworld::GetClient()->SendSoundMessageToConversation(TCHAR_TO_UTF8(*ConversationId), data);
}

void FInworldClient::SendSoundMessageWithEAC(const FString& AgentId, class USoundWave* Input, class USoundWave* Output)
{
	std::vector<int16_t> inputdata, outputdata;
	if (Inworld::Utils::SoundWaveToVec(Input, inputdata) && Inworld::Utils::SoundWaveToVec(Output, outputdata))
	{
		Inworld::GetClient()->SendSoundMessageWithAEC(TCHAR_TO_UTF8(*AgentId), inputdata, outputdata);
	}
}

void FInworldClient::SendSoundMessageWithEACToConversation(const FString& ConversationId, USoundWave* Input,
	USoundWave* Output)
{
	std::vector<int16_t> inputdata, outputdata;
	if (Inworld::Utils::SoundWaveToVec(Input, inputdata) && Inworld::Utils::SoundWaveToVec(Output, outputdata))
	{
		Inworld::GetClient()->SendSoundMessageWithAECToConversation(TCHAR_TO_UTF8(*ConversationId), inputdata, outputdata);
	}
}

void FInworldClient::SendSoundDataMessageWithEAC(const FString& AgentId, const TArray<uint8>& InputData, const TArray<uint8>& OutputData)
{
	std::vector<int16> inputdata((int16*)InputData.GetData(), ((int16*)InputData.GetData()) + (InputData.Num() / 2));
	std::vector<int16> outputdata((int16*)OutputData.GetData(), ((int16*)OutputData.GetData()) + (OutputData.Num() / 2));
	Inworld::GetClient()->SendSoundMessageWithAEC(TCHAR_TO_UTF8(*AgentId), inputdata, outputdata);
}

void FInworldClient::SendSoundDataMessageWithEACToConversation(const FString& ConversationId,
	const TArray<uint8>& InputData, const TArray<uint8>& OutputData)
{
	std::vector<int16> inputdata((int16*)InputData.GetData(), ((int16*)InputData.GetData()) + (InputData.Num() / 2));
	std::vector<int16> outputdata((int16*)OutputData.GetData(), ((int16*)OutputData.GetData()) + (OutputData.Num() / 2));
	Inworld::GetClient()->SendSoundMessageWithAECToConversation(TCHAR_TO_UTF8(*ConversationId), inputdata, outputdata);
}

void FInworldClient::StartAudioSession(const FString& AgentId)
{
	Inworld::GetClient()->StartAudioSession(TCHAR_TO_UTF8(*AgentId));
}

void FInworldClient::StartAudioSessionInConversation(const FString& ConversationId)
{
	Inworld::GetClient()->StartAudioSessionInConversation(TCHAR_TO_UTF8(*ConversationId));
}

void FInworldClient::StopAudioSession(const FString& AgentId)
{
	Inworld::GetClient()->StopAudioSession(TCHAR_TO_UTF8(*AgentId));
}

void FInworldClient::StopAudioSessionInConversation(const FString& ConversationId)
{
	Inworld::GetClient()->StopAudioSessionInConversation(TCHAR_TO_UTF8(*ConversationId));
}

std::unordered_map<std::string, std::string> ToStd(const TMap<FString, FString>& Map)
{
	std::unordered_map<std::string, std::string> StdMap;
	for (const TPair<FString, FString>& Entry : Map)
	{
		StdMap.insert(std::make_pair<std::string, std::string>(TCHAR_TO_UTF8(*Entry.Key), TCHAR_TO_UTF8(*Entry.Value)));
	}
	return StdMap;
}

void FInworldClient::SendCustomEvent(const FString& AgentId, const FString& Name, const TMap<FString, FString>& Params)
{
	Inworld::GetClient()->SendCustomEvent(TCHAR_TO_UTF8(*AgentId), TCHAR_TO_UTF8(*Name), ToStd(Params));
}

void FInworldClient::SendChangeSceneEvent(const FString& SceneName)
{
	Inworld::GetClient()->LoadScene(TCHAR_TO_UTF8(*SceneName));
}

void FInworldClient::SendNarrationEvent(const FString& AgentId, const FString& Content)
{
	Inworld::GetClient()->SendNarrationEvent(TCHAR_TO_UTF8(*AgentId), TCHAR_TO_UTF8(*Content));
}

void FInworldClient::CancelResponse(const FString& AgentId, const FString& InteractionId, const TArray<FString>& UtteranceIds)
{
	std::vector<std::string> utteranceIds;
	utteranceIds.reserve(UtteranceIds.Num());
	for (auto& Id : UtteranceIds)
	{
		utteranceIds.push_back(TCHAR_TO_UTF8(*Id));
	}

	Inworld::GetClient()->CancelResponse(TCHAR_TO_UTF8(*AgentId), TCHAR_TO_UTF8(*InteractionId), utteranceIds);
}

#if !UE_BUILD_SHIPPING
void FInworldClient::OnCVarsChanged()
{
	static bool GEnableSoundDump = CVarEnableSoundDump.GetValueOnGameThread();
	static FString GSoundDumpPath = CVarSoundDumpPath.GetValueOnGameThread();

	bool bNewEnableSoundDump = CVarEnableSoundDump.GetValueOnGameThread();
	FString NewSoundDumpPath = CVarSoundDumpPath.GetValueOnGameThread();
	if (GEnableSoundDump != bNewEnableSoundDump || GSoundDumpPath != NewSoundDumpPath)
	{
		GEnableSoundDump = bNewEnableSoundDump;
		GSoundDumpPath = NewSoundDumpPath;

		OnAudioDumperCVarChanged.Broadcast(GEnableSoundDump, GSoundDumpPath);
	}
}
#endif
