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
#include "InworldAsyncRoutine.h"
#include "InworldPacketTranslator.h"

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

namespace Inworld
{
	class FClient
	{
	public:
		FClient()
		{	
			//_Client.CreateAsyncRoutines<FInworldAsyncRoutine>();
		}

		void StartClient(const ClientOptions& Options, const SessionInfo& Info, CharactersLoadedCb LoadSceneCallback)
		{
			_Client.StartClient(Options, Info, LoadSceneCallback, [this, SelfPtr = SelfWeakPtr](std::function<void()> Task)
				{
					AsyncTask(ENamedThreads::GameThread, [Task, SelfPtr]() {
						if (SelfPtr.IsValid())
						{
							Task();
						}
						});
				});
		}

		Inworld::Client& Client() { return _Client; }

	protected:

	public:
		Inworld::Client _Client;
		TWeakPtr<FClient> SelfWeakPtr;
	};
}

void FInworldClient::Init()
{
	FString ClientVer;
	TSharedPtr<IPlugin> InworldAIPlugin = IPluginManager::Get().FindPlugin("InworldAI");
	if (ensure(InworldAIPlugin.IsValid()))
	{
		ClientVer = InworldAIPlugin.Get()->GetDescriptor().VersionName;
	}
	FString ClientId("unreal");

	int64 Size0 = Inworld::SizeOfSdkInfo();
	int64 Size1 = sizeof(Inworld::SdkInfo);

	Inworld::SdkInfo Sdk;
	Sdk.Type = TCHAR_TO_UTF8(*ClientId);
	Sdk.Version = TCHAR_TO_UTF8(*ClientVer);
	
	FString OSVersion, OSSubversion;
	FPlatformMisc::GetOSVersions(OSVersion, OSSubversion);
	FString OSFullVersion = FString::Printf(TEXT("%s %s"), *OSVersion, *OSSubversion);
	Sdk.OS = TCHAR_TO_UTF8(*OSFullVersion);

	InworldClient = MakeShared<Inworld::FClient>();
	InworldClient->SelfWeakPtr = InworldClient;
	InworldClient->Client().InitClient(Sdk,
		[this](Inworld::Client::ConnectionState ConnectionState)
		{
			OnConnectionStateChanged.ExecuteIfBound(static_cast<EInworldConnectionState>(ConnectionState));
		},
		[this](std::shared_ptr<Inworld::Packet> Packet)
		{
			InworldPacketTranslator PacketTranslator;
			Packet->Accept(PacketTranslator);
			TSharedPtr<FInworldPacket> ReceivedPacket = PacketTranslator.GetPacket();
			if (ReceivedPacket.IsValid())
			{
				OnInworldPacketReceived.ExecuteIfBound(ReceivedPacket);
			}
		}
	);

	InworldClient->Client().SetPerceivedLatencyTrackerCallback([this](const std::string& InteractionId, uint32_t LatancyMs)
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
			InworldClient->Client().SetAudioDumpEnabled(false, "");
			return;
		}
		const std::string StdPath = TCHAR_TO_UTF8(*DumpPath);
		InworldClient->Client().SetAudioDumpEnabled(false, StdPath);

		if (bEnable)
		{
			UE_LOG(LogInworldAIClient, Log, TEXT("Audio dump path: %s."), *DumpPath);
			InworldClient->Client().SetAudioDumpEnabled(true, StdPath);
		}
	};
	OnAudioDumperCVarChangedHandle = OnAudioDumperCVarChanged.AddLambda(OnAudioDumperCVarChangedCallback);
	OnAudioDumperCVarChangedCallback(CVarEnableSoundDump.GetValueOnGameThread(), CVarSoundDumpPath.GetValueOnGameThread());
#endif
}

void FInworldClient::Destroy()
{
#if !UE_BUILD_SHIPPING
	OnAudioDumperCVarChanged.Remove(OnAudioDumperCVarChangedHandle);
#endif
	if (InworldClient)
	{
		InworldClient->Client().DestroyClient();
	}
	InworldClient.Reset();
}

void FInworldClient::Start(const FString& SceneName, const FInworldPlayerProfile& PlayerProfile, const FInworldCapabilitySet& Capabilities, const FInworldAuth& Auth, const FInworldSessionToken& SessionToken, const FInworldSave& Save, const FInworldEnvironment& Environment)
{
	/*Inworld::ClientOptions Options;
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
	Options.PlayerName = TCHAR_TO_UTF8(*PlayerProfile.Name);
	Options.ProjectName = TCHAR_TO_UTF8(*PlayerProfile.ProjectName);
	Options.UserId = PlayerProfile.UniqueId.IsEmpty() ? TCHAR_TO_UTF8(*GenerateUserId()) : TCHAR_TO_UTF8(*PlayerProfile.UniqueId);
	Options.UserSettings.Profile.Fields.reserve(PlayerProfile.Fields.Num());
	for (const auto& ProfileField : PlayerProfile.Fields)
	{
		Inworld::UserSettings::PlayerProfile::PlayerField PlayerField;
		PlayerField.Id = TCHAR_TO_UTF8(*ProfileField.Key);
		PlayerField.Value = TCHAR_TO_UTF8(*ProfileField.Value);
		Options.UserSettings.Profile.Fields.push_back(PlayerField);
	}

	Options.Capabilities.Animations = Capabilities.Animations;
	Options.Capabilities.Audio = Capabilities.Audio;
	Options.Capabilities.Emotions = Capabilities.Emotions;
	Options.Capabilities.Interruptions = Capabilities.Interruptions;
	Options.Capabilities.EmotionStreaming = Capabilities.EmotionStreaming;
	Options.Capabilities.SilenceEvents = Capabilities.SilenceEvents;
	Options.Capabilities.PhonemeInfo = Capabilities.PhonemeInfo;
	Options.Capabilities.Continuation = Capabilities.Continuation;
	Options.Capabilities.TurnBasedSTT = Capabilities.TurnBasedSTT;
	Options.Capabilities.NarratedActions = Capabilities.NarratedActions;
	Options.Capabilities.Relations = Capabilities.Relations;
	Options.Capabilities.Multiagent = Capabilities.MultiAgent;

	Inworld::SessionInfo Info;
	Info.Token = TCHAR_TO_UTF8(*SessionToken.Token);
	Info.ExpirationTime = SessionToken.ExpirationTime;
	Info.SessionId = TCHAR_TO_UTF8(*SessionToken.SessionId);

	if (Save.Data.Num() != 0)
    {
        Info.SessionSavedState.resize(Save.Data.Num());
        FMemory::Memcpy((uint8*)Info.SessionSavedState.data(), (uint8*)Save.Data.GetData(), Info.SessionSavedState.size());
    }

	InworldClient->StartClient(Options, Info,
		[this](const std::vector<Inworld::AgentInfo>& ResultAgentInfos)
		{
			TArray<FInworldAgentInfo> AgentInfos;
			AgentInfos.Reserve(ResultAgentInfos.size());
			for (const auto& ResultAgentInfo : ResultAgentInfos)
			{
				auto& AgentInfo = AgentInfos.AddDefaulted_GetRef();
				AgentInfo.AgentId = UTF8_TO_TCHAR(ResultAgentInfo.AgentId.c_str());
				AgentInfo.BrainName = UTF8_TO_TCHAR(ResultAgentInfo.BrainName.c_str());
				AgentInfo.GivenName = UTF8_TO_TCHAR(ResultAgentInfo.GivenName.c_str());
			}
			OnSceneLoaded.ExecuteIfBound(AgentInfos);
		}
	);*/
}

void FInworldClient::Stop()
{
	InworldClient->Client().StopClient();
}

void FInworldClient::Pause()
{
	InworldClient->Client().PauseClient();
}

void FInworldClient::Resume()
{
	InworldClient->Client().ResumeClient();
}

void FInworldClient::SaveSession()
{
	InworldClient->Client().SaveSessionState([this](std::string Data, bool bSuccess)
		{
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
}

FString FInworldClient::GenerateUserId()
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
	SId = Inworld::Utils::ToHex(Data);

	return FString(UTF8_TO_TCHAR(Inworld::Utils::ToHex(Data).c_str()));
}

EInworldConnectionState FInworldClient::GetConnectionState() const
{
	return InworldClient ? static_cast<EInworldConnectionState>(InworldClient->Client().GetConnectionState()) : EInworldConnectionState::Disconnected;
}

void FInworldClient::GetConnectionError(FString& OutErrorMessage, int32& OutErrorCode) const
{
	std::string OutError;
	InworldClient->Client().GetConnectionError(OutError, OutErrorCode);
	OutErrorMessage = UTF8_TO_TCHAR(OutError.c_str());
}

FString FInworldClient::GetSessionId() const
{
	return UTF8_TO_TCHAR(Inworld::GetSessionId().c_str());
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

TSharedPtr<FInworldPacket> FInworldClient::SendTextMessage(const TArray<FString>& AgentIds, const FString& Text)
{
	auto Packet = InworldClient->Client().SendTextMessage(ToStd(AgentIds), TCHAR_TO_UTF8(*Text));
	InworldPacketTranslator PacketTranslator;
	Packet->Accept(PacketTranslator);
	return PacketTranslator.GetPacket();
}

void FInworldClient::SendSoundMessage(const TArray<FString>& AgentIds, class USoundWave* Sound)
{
	std::string data;
	if (Inworld::Utils::SoundWaveToString(Sound, data))
	{
		InworldClient->Client().SendSoundMessage(ToStd(AgentIds), data);
	}
}

void FInworldClient::SendSoundDataMessage(const TArray<FString>& AgentIds, const TArray<uint8>& Data)
{
	std::string data((char*)Data.GetData(), Data.Num());
	InworldClient->Client().SendSoundMessage(ToStd(AgentIds), data);
}

void FInworldClient::SendSoundMessageWithEAC(const TArray<FString>& AgentIds, class USoundWave* Input, class USoundWave* Output)
{
	std::vector<int16_t> inputdata, outputdata;
	if (Inworld::Utils::SoundWaveToVec(Input, inputdata) && Inworld::Utils::SoundWaveToVec(Output, outputdata))
	{
		InworldClient->Client().SendSoundMessageWithAEC(ToStd(AgentIds), inputdata, outputdata);
	}
}

void FInworldClient::SendSoundDataMessageWithEAC(const TArray<FString>& AgentIds, const TArray<uint8>& InputData, const TArray<uint8>& OutputData)
{
	std::vector<int16> inputdata((int16*)InputData.GetData(), ((int16*)InputData.GetData()) + (InputData.Num() / 2));
	std::vector<int16> outputdata((int16*)OutputData.GetData(), ((int16*)OutputData.GetData()) + (OutputData.Num() / 2));
	InworldClient->Client().SendSoundMessageWithAEC(ToStd(AgentIds), inputdata, outputdata);
}

void FInworldClient::StartAudioSession(const TArray<FString>& AgentIds)
{
	InworldClient->Client().StartAudioSession(ToStd(AgentIds));
}

void FInworldClient::StopAudioSession(const TArray<FString>& AgentIds)
{
	InworldClient->Client().StopAudioSession(ToStd(AgentIds));
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

void FInworldClient::SendCustomEvent(const TArray<FString>& AgentIds, const FString& Name, const TMap<FString, FString>& Params)
{
	InworldClient->Client().SendCustomEvent(ToStd(AgentIds), TCHAR_TO_UTF8(*Name), ToStd(Params));
}

void FInworldClient::SendChangeSceneEvent(const FString& SceneName)
{
	InworldClient->Client().LoadScene(TCHAR_TO_UTF8(*SceneName), nullptr);
}

void FInworldClient::SendNarrationEvent(const FString& AgentId, const FString& Content)
{
	InworldClient->Client().SendNarrationEvent(TCHAR_TO_UTF8(*AgentId), TCHAR_TO_UTF8(*Content));
}

void FInworldClient::CancelResponse(const FString& AgentId, const FString& InteractionId, const TArray<FString>& UtteranceIds)
{
	std::vector<std::string> utteranceIds;
	utteranceIds.reserve(UtteranceIds.Num());
	for (auto& Id : UtteranceIds)
	{
		utteranceIds.push_back(TCHAR_TO_UTF8(*Id));
	}

	InworldClient->Client().CancelResponse(TCHAR_TO_UTF8(*AgentId), TCHAR_TO_UTF8(*InteractionId), utteranceIds);
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
