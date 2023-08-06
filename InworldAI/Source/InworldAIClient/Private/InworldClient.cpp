/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldClient.h"
#include "CoreMinimal.h"

#include "InworldUtils.h"

#include "Async/Async.h"
#include "Async/TaskGraphInterfaces.h"

#include "NDK/Utils/Log.h"

#include <string>

#if !UE_BUILD_SHIPPING
static TAutoConsoleVariable<bool> CVarEnableSoundDump(
	TEXT("Inworld.Debug.EnableSoundDump"), false,
	TEXT("Enable/Disable recording audio input to dump file")
);

static TAutoConsoleVariable<FString> CVarSoundDumpPath(
	TEXT("Inworld.Debug.SoundDumpPath"), TEXT("C:/Tmp/AudioDump.wav"),
	TEXT("Specifiy path for audio input dump file")
);

Inworld::FClient::FOnAudioDumperCVarChanged Inworld::FClient::OnAudioDumperCVarChanged;

FAutoConsoleVariableSink Inworld::FClient::CVarSink(FConsoleCommandDelegate::CreateStatic(&Inworld::FClient::OnCVarsChanged));
#endif

void Inworld::FClient::InitClient(std::string ClientId, std::string ClientVer, std::function<void(ConnectionState)> ConnectionStateCallback, std::function<void(std::shared_ptr<Inworld::Packet>)> PacketCallback)
{
	ClientBase::InitClient(ClientId, ClientVer, ConnectionStateCallback, PacketCallback);
#if !UE_BUILD_SHIPPING
	auto OnAudioDumperCVarChangedCallback = [this](bool bEnable, FString Path)
	{
		AsyncAudioDumper.Stop();
		if (bEnable)
		{
			AsyncAudioDumper.Start("InworldAudioDumper", std::make_unique<FRunnableAudioDumper>(AudioChunksToDump, Path));
		}
	};
	OnAudioDumperCVarChangedHandle = OnAudioDumperCVarChanged.AddLambda(OnAudioDumperCVarChangedCallback);
	OnAudioDumperCVarChangedCallback(CVarEnableSoundDump.GetValueOnGameThread(), CVarSoundDumpPath.GetValueOnGameThread());
#endif
}

void Inworld::FClient::StartClient(const ClientOptions& Options, const SessionInfo& Info, std::function<void(const std::vector<AgentInfo>&)> LoadSceneCallback)
{
	if (Options.UserId.empty())
	{
		ClientOptions Opt = Options;
		Opt.UserId = GenerateUserId();
		Inworld::ClientBase::StartClient(Opt, Info, LoadSceneCallback);
	}
	else
	{
		Inworld::ClientBase::StartClient(Options, Info, LoadSceneCallback);
	}
}

void Inworld::FClient::DestroyClient()
{
#if !UE_BUILD_SHIPPING
	AsyncAudioDumper.Stop();
	OnAudioDumperCVarChanged.Remove(OnAudioDumperCVarChangedHandle);
#endif
	ClientBase::DestroyClient();
}

std::shared_ptr<Inworld::DataEvent> Inworld::FClient::SendSoundMessage(const std::string& AgentId, const std::string& Data)
{
	auto Packet = ClientBase::SendSoundMessage(AgentId, Data);
#if !UE_BUILD_SHIPPING
	if (CVarEnableSoundDump.GetValueOnGameThread())
	{
		AudioChunksToDump.Enqueue(Data);
	}
#endif
	return Packet;
}

void Inworld::FClient::AddTaskToMainThread(std::function<void()> Task)
{
	AsyncTask(ENamedThreads::GameThread, [Task, SelfPtr = SelfWeakPtr]() {
		if (!SelfPtr.expired())
		{
			Task();
		}
	});
}

std::string Inworld::FClient::GenerateUserId()
{
	FString Id = FPlatformMisc::GetDeviceId();
	if (Id.IsEmpty())
	{
		Id = FPlatformMisc::GetMacAddressString();
	}
	if (Id.IsEmpty())
	{
		Inworld::LogError("Couldn't generate user id.");
		return std::string();
	}

	Inworld::Log("Device Id: %s", *Id);

	std::string SId = TCHAR_TO_UTF8(*Id);
	TArray<uint8> Data;
	Data.SetNumZeroed(SId.size());
	FMemory::Memcpy(Data.GetData(), SId.data(), SId.size());

	Data = Inworld::Utils::HmacSha256(Data, Data);
	SId = Utils::ToHex(Data);

	return Utils::ToHex(Data);
}

#if !UE_BUILD_SHIPPING
void Inworld::FClient::OnCVarsChanged()
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

#if !UE_BUILD_SHIPPING
void Inworld::FRunnableAudioDumper::Run()
{
    AudioDumper.OnSessionStart(TCHAR_TO_UTF8(*FileName));

    while (!_IsDone)
    {
        FPlatformProcess::Sleep(0.1f);

		std::string Chunk;
		while (AudioChuncks.Dequeue(Chunk))
		{
			AudioDumper.OnMessage(Chunk);
		}
    }

    AudioDumper.OnSessionStop();
}
#endif
