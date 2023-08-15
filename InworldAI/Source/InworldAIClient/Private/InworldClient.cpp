/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldClient.h"
#include "CoreMinimal.h"

#include "SocketSubsystem.h"
#include "IPAddress.h"

#include "InworldUtils.h"
#include "InworldAsyncRoutine.h"
#include "InworldPacketTranslator.h"

THIRD_PARTY_INCLUDES_START
#include "Packets.h"
#include "Client.h"
#include "AsyncRoutine.h"
#include "RunnableCommand.h"
#include "Utils/Log.h"
THIRD_PARTY_INCLUDES_END

#include "Async/Async.h"
#include "Async/TaskGraphInterfaces.h"

#include <Interfaces/IPluginManager.h>

const FString DefaultTargetUrl = "api-engine.inworld.ai:443";

#include <string>

#if !UE_BUILD_SHIPPING

#include "AudioSessionDumper.h"

static TAutoConsoleVariable<bool> CVarEnableSoundDump(
	TEXT("Inworld.Debug.EnableSoundDump"), false,
	TEXT("Enable/Disable recording audio input to dump file")
);

static TAutoConsoleVariable<FString> CVarSoundDumpPath(
	TEXT("Inworld.Debug.SoundDumpPath"), TEXT("C:/Tmp/AudioDump.wav"),
	TEXT("Specifiy path for audio input dump file")
);

FInworldClient::FOnAudioDumperCVarChanged FInworldClient::OnAudioDumperCVarChanged;

FAutoConsoleVariableSink FInworldClient::CVarSink(FConsoleCommandDelegate::CreateStatic(&FInworldClient::OnCVarsChanged));
#endif

namespace Inworld
{
	class FClient : public ClientBase
	{
	public:
		FClient()
		{	
			class FClientAsyncRoutine : public IAsyncRoutine
			{
			public:
				virtual void Start(std::string ThreadName, std::unique_ptr<Runnable> Runnable) override
				{
					AsyncRoutinePtr = MakeShared<AsyncRoutine>(UTF8_TO_TCHAR(ThreadName.c_str()), std::move(Runnable));
					AsyncRoutinePtr->Start();
				}
				virtual void Stop() override { if(AsyncRoutinePtr) AsyncRoutinePtr->Stop(); }
				virtual bool IsDone() const { return AsyncRoutinePtr ? AsyncRoutinePtr->IsDone() : true; }
				virtual bool IsValid() const { return AsyncRoutinePtr ? AsyncRoutinePtr->IsValid() : false; }
				virtual Inworld::Runnable* GetRunnable() { return AsyncRoutinePtr ? AsyncRoutinePtr->GetRunnable() : nullptr; }

			private:
				struct Runnable : public FInworldRunnable<std::unique_ptr<Inworld::Runnable>>
				{
					Runnable(std::unique_ptr<Inworld::Runnable> InRunnable)
					{
						RunnablePtr = std::move(InRunnable);
					}
				};

				struct AsyncRoutine : public Inworld::FAsyncRoutine<Runnable>
				{
					AsyncRoutine(FString InThreadName, std::unique_ptr<Inworld::Runnable> InRunnable)
					{
						ThreadName = InThreadName;
						RunnablePtr = MakeUnique<Runnable>(MoveTemp(InRunnable));
					}
				};

				TSharedPtr<AsyncRoutine> AsyncRoutinePtr;
			};

			CreateAsyncRoutines<FClientAsyncRoutine>();
		}
	protected:

		virtual void AddTaskToMainThread(std::function<void()> Task) override
		{
			AsyncTask(ENamedThreads::GameThread, [Task, SelfPtr = SelfWeakPtr]() {
				if (SelfPtr.IsValid())
				{
					Task();
				}
			});
		}

	public:
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
	InworldClient = MakeShared<Inworld::FClient>();
	InworldClient->SelfWeakPtr = InworldClient;
	InworldClient->InitClient(TCHAR_TO_UTF8(*ClientId), TCHAR_TO_UTF8(*ClientVer),
		[this](Inworld::ClientBase::ConnectionState ConnectionState)
		{
			OnConnectionStateChanged.ExecuteIfBound(static_cast<EInworldConnectionState>(ConnectionState));
		},
		[this](std::shared_ptr<Inworld::Packet> Packet)
		{
			InworldPacketTranslator PacketTranslator;
			Packet->Accept(PacketTranslator);
			OnInworldPacketReceived.ExecuteIfBound(PacketTranslator.GetPacket());
		}
	);
#if !UE_BUILD_SHIPPING
	auto OnAudioDumperCVarChangedCallback = [this](bool bEnable, FString Path)
	{
		if (AsyncAudioDumper.IsValid())
		{
			AsyncAudioDumper->Stop();
			AsyncAudioDumper.Reset();
		}

		if (bEnable)
		{
			AsyncAudioDumper = MakeShared<FAsyncAudioDumper>();
			AsyncAudioDumper->Start(Path);
		}
	};
	OnAudioDumperCVarChangedHandle = OnAudioDumperCVarChanged.AddLambda(OnAudioDumperCVarChangedCallback);
	OnAudioDumperCVarChangedCallback(CVarEnableSoundDump.GetValueOnGameThread(), CVarSoundDumpPath.GetValueOnGameThread());
#endif
}

void FInworldClient::Destroy()
{
#if !UE_BUILD_SHIPPING
	if (AsyncAudioDumper.IsValid())
	{
		AsyncAudioDumper->Stop();
		AsyncAudioDumper.Reset();
	}
	OnAudioDumperCVarChanged.Remove(OnAudioDumperCVarChangedHandle);
#endif
	if (InworldClient)
	{
		InworldClient->DestroyClient();
	}
	InworldClient.Reset();
}

void FInworldClient::Start(const FString& SceneName, const FInworldPlayerProfile& PlayerProfile, const FInworldCapabilitySet& Capabilities, const FInworldAuth& Auth, const FInworldSessionToken& SessionToken, const FInworldEnvironment& Environment)
{
	Inworld::ClientOptions Options;
	Options.ServerUrl = TCHAR_TO_UTF8(*(!Environment.TargetUrl.IsEmpty() ? Environment.TargetUrl : DefaultTargetUrl));
	Options.SceneName = TCHAR_TO_UTF8(*SceneName);
	Options.ApiKey = TCHAR_TO_UTF8(*Auth.ApiKey);
	Options.ApiSecret = TCHAR_TO_UTF8(*Auth.ApiSecret);
	Options.PlayerName = TCHAR_TO_UTF8(*PlayerProfile.Name);
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
	Options.Capabilities.Text = Capabilities.Text;
	Options.Capabilities.Audio = Capabilities.Audio;
	Options.Capabilities.Emotions = Capabilities.Emotions;
	Options.Capabilities.Gestures = Capabilities.Gestures;
	Options.Capabilities.Interruptions = Capabilities.Interruptions;
	Options.Capabilities.Triggers = Capabilities.Triggers;
	Options.Capabilities.EmotionStreaming = Capabilities.EmotionStreaming;
	Options.Capabilities.SilenceEvents = Capabilities.SilenceEvents;
	Options.Capabilities.PhonemeInfo = Capabilities.PhonemeInfo;
	Options.Capabilities.LoadSceneInSession = Capabilities.LoadSceneInSession;

	Inworld::SessionInfo Info;
	Info.Token = TCHAR_TO_UTF8(*SessionToken.Token);
	Info.ExpirationTime = SessionToken.ExpirationTime;
	Info.SessionId = TCHAR_TO_UTF8(*SessionToken.SessionId);

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
	);
}

void FInworldClient::Stop()
{
	InworldClient->StopClient();
}

void FInworldClient::Pause()
{
	InworldClient->PauseClient();
}

void FInworldClient::Resume()
{
	InworldClient->ResumeClient();
}

FString FInworldClient::GenerateUserId()
{
	FString Id = FPlatformMisc::GetDeviceId();
	if (Id.IsEmpty())
	{
		Id = FPlatformMisc::GetMacAddressString();
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
	return static_cast<EInworldConnectionState>(InworldClient->GetConnectionState());
}

void FInworldClient::GetConnectionError(FString& OutErrorMessage, int32& OutErrorCode) const
{
	std::string OutError;
	InworldClient->GetConnectionError(OutError, OutErrorCode);
	OutErrorMessage = UTF8_TO_TCHAR(OutError.c_str());
}

FString FInworldClient::GetSessionId() const
{
	return UTF8_TO_TCHAR(Inworld::g_SessionId.c_str());
}

TSharedPtr<FInworldPacket> FInworldClient::SendTextMessage(const FString& AgentId, const FString& Text)
{
	auto Packet = InworldClient->SendTextMessage(TCHAR_TO_UTF8(*AgentId), TCHAR_TO_UTF8(*Text));
	InworldPacketTranslator PacketTranslator;
	Packet->Accept(PacketTranslator);
	return PacketTranslator.GetPacket();
}

#if !UE_BUILD_SHIPPING
void DumpAudio(TSharedPtr<class FAsyncAudioDumper> AudioDumper, std::shared_ptr<Inworld::DataEvent> DataEvent)
{
	if (AudioDumper.IsValid())
	{
		std::string data = DataEvent->GetDataChunk();
		TArray<uint8> Chunk((uint8*)data.data(), data.size());
		AudioDumper->QueueChunk(Chunk);
	}
}
#endif

void FInworldClient::SendSoundMessage(const FString& AgentId, USoundWave* Sound)
{
	std::string data;
	if (Inworld::Utils::SoundWaveToString(Sound, data))
	{
		auto packet = InworldClient->SendSoundMessage(TCHAR_TO_UTF8(*AgentId), data);
#if !UE_BUILD_SHIPPING
		DumpAudio(AsyncAudioDumper, packet);
#endif
	}
}

void FInworldClient::SendSoundDataMessage(const FString& AgentId, const TArray<uint8>& Data)
{
	std::string data((char*)Data.GetData(), Data.Num());
	auto packet = InworldClient->SendSoundMessage(TCHAR_TO_UTF8(*AgentId), data);
#if !UE_BUILD_SHIPPING
	DumpAudio(AsyncAudioDumper, packet);
#endif
}

void FInworldClient::SendSoundMessageWithEAC(const FString& AgentId, USoundWave* Input, USoundWave* Output)
{
	std::vector<int16_t> inputdata, outputdata;
	if (Inworld::Utils::SoundWaveToVec(Input, inputdata) && Inworld::Utils::SoundWaveToVec(Output, outputdata))
	{
		auto packet = InworldClient->SendSoundMessageWithAEC(TCHAR_TO_UTF8(*AgentId), inputdata, outputdata);
#if !UE_BUILD_SHIPPING
		DumpAudio(AsyncAudioDumper, packet);
#endif
	}
}

void FInworldClient::SendSoundDataMessageWithEAC(const FString& AgentId, const TArray<uint8>& InputData, const TArray<uint8>& OutputData)
{
	std::vector<int16> inputdata((int16*)InputData.GetData(), ((int16*)InputData.GetData()) + (InputData.Num() / 2));
	std::vector<int16> outputdata((int16*)OutputData.GetData(), ((int16*)OutputData.GetData()) + (OutputData.Num() / 2));
	auto packet = InworldClient->SendSoundMessageWithAEC(TCHAR_TO_UTF8(*AgentId), inputdata, outputdata);
#if !UE_BUILD_SHIPPING
	DumpAudio(AsyncAudioDumper, packet);
#endif
}

void FInworldClient::StartAudioSession(const FString& AgentId)
{
	InworldClient->StartAudioSession(TCHAR_TO_UTF8(*AgentId));
}

void FInworldClient::StopAudioSession(const FString& AgentId)
{
	InworldClient->StopAudioSession(TCHAR_TO_UTF8(*AgentId));
}

void FInworldClient::SendCustomEvent(const FString& AgentId, const FString& Name, const TMap<FString, FString>& Params)
{
	std::unordered_map<std::string, std::string> params;

	for (const TPair<FString, FString>& Param : Params)
	{
		params.insert(std::make_pair<std::string, std::string>(TCHAR_TO_UTF8(*Param.Key), TCHAR_TO_UTF8(*Param.Value)));
	}

	InworldClient->SendCustomEvent(TCHAR_TO_UTF8(*AgentId), TCHAR_TO_UTF8(*Name), params);
}

void FInworldClient::SendChangeSceneEvent(const FString& SceneName)
{
	InworldClient->SendChangeSceneEvent(TCHAR_TO_UTF8(*SceneName));
}

void FInworldClient::CancelResponse(const FString& AgentId, const FString& InteractionId, const TArray<FString>& UtteranceIds)
{
	std::vector<std::string> utteranceIds;
	utteranceIds.reserve(UtteranceIds.Num());
	for (auto& Id : UtteranceIds)
	{
		utteranceIds.push_back(TCHAR_TO_UTF8(*Id));
	}

	InworldClient->CancelResponse(TCHAR_TO_UTF8(*AgentId), TCHAR_TO_UTF8(*InteractionId), utteranceIds);
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
