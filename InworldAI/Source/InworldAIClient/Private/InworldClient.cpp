/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldClient.h"
#include "InworldAIClientModule.h"
#include "InworldMacros.h"
#include "CoreMinimal.h"
#include "GenericPlatform/GenericPlatformMisc.h"

#include "SocketSubsystem.h"
#include "IPAddress.h"

#include "InworldAINDKModule.h"
#include "InworldUtils.h"
#include "InworldPacketTranslator.h"
#include "ThirdParty/InworldAINDKLibrary/include/InworldVAD.h"

//#include "onnxruntime_cxx_api.h"

THIRD_PARTY_INCLUDES_START
#include "Packets.h"
#include "Client.h"
#include "Utils/Log.h"
THIRD_PARTY_INCLUDES_END

#include "Async/Async.h"
#include "Async/TaskGraphInterfaces.h"

#include <Interfaces/IPluginManager.h>

#include "Misc/Paths.h"
#include "Misc/App.h"

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

UInworldClient::FOnAudioDumperCVarChanged UInworldClient::OnAudioDumperCVarChanged;

FAutoConsoleVariableSink UInworldClient::CVarSink(FConsoleCommandDelegate::CreateStatic(&UInworldClient::OnCVarsChanged));
#endif

#define EMPTY_ARG_RETURN(Arg, Return) INWORLD_WARN_AND_RETURN_EMPTY(LogInworldAIClient, UInworldClient, Arg, Return)
#define NO_CLIENT_RETURN(Return) EMPTY_ARG_RETURN(Inworld::GetClient(), Return)

std::vector<std::string> ToStd(const TArray<FString>& Array)
{
	std::vector<std::string> Vec;
	for (auto& Str : Array)
	{
		Vec.emplace_back(TCHAR_TO_UTF8(*Str));
	}
	return Vec;
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

UInworldClient::UInworldClient()
	: Super()
{
	// Ensure dependencies are loaded
	FInworldAINDKModule::Get();

	AudioSender = CreateDefaultSubobject<UInworldAudioSender>(TEXT("AudioSender"));

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
					if (bIsBeingDestroyed)
					{
						return;
					}
					OnConnectionStateChangedDelegateNative.Broadcast(static_cast<EInworldConnectionState>(ConnectionState));
					OnConnectionStateChangedDelegate.Broadcast(static_cast<EInworldConnectionState>(ConnectionState));
				});
		},
		[this](std::shared_ptr<Inworld::Packet> Packet)
		{
			if (bIsBeingDestroyed)
			{
				return;
			}
			AsyncTask(ENamedThreads::GameThread, [this, Packet]()
				{
					if (bIsBeingDestroyed)
					{
						return;
					}
					InworldPacketTranslator PacketTranslator;
					Packet->Accept(PacketTranslator);
					TSharedPtr<FInworldPacket> ReceivedPacket = PacketTranslator.GetPacket();
					if (ReceivedPacket.IsValid())
					{
						OnPacketReceivedDelegateNative.Broadcast(ReceivedPacket);
						OnPacketReceivedDelegate.Broadcast(ReceivedPacket);
					}
				});
		}
	);

	Inworld::GetClient()->SetPerceivedLatencyTrackerCallback([this](const std::string& InteractionId, uint32_t LatencyMs)
		{
			OnPerceivedLatencyDelegateNative.Broadcast(UTF8_TO_TCHAR(InteractionId.c_str()), LatencyMs);
			OnPerceivedLatencyDelegate.Broadcast(UTF8_TO_TCHAR(InteractionId.c_str()), LatencyMs);
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

UInworldClient::~UInworldClient()
{
	bIsBeingDestroyed = true;
#if !UE_BUILD_SHIPPING
	OnAudioDumperCVarChanged.Remove(OnAudioDumperCVarChangedHandle);
#endif
	if (IsValid(AudioSender))
	{
#if ENGINE_MAJOR_VERSION == 5
		AudioSender->MarkAsGarbage();
#endif
#if ENGINE_MAJOR_VERSION == 4
		AudioSender->MarkPendingKill();
#endif
	}
	AudioSender = nullptr;
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

void UInworldClient::StartSession(const FString& SceneId, const FInworldPlayerProfile& PlayerProfile, const FInworldAuth& Auth, const FInworldSave& Save, const FInworldSessionToken& SessionToken, const FInworldCapabilitySet& CapabilitySet)
{
	NO_CLIENT_RETURN(void())

	Inworld::ClientOptions Options;
	Options.ServerUrl = TCHAR_TO_UTF8(*(!Environment.TargetUrl.IsEmpty() ? Environment.TargetUrl : DefaultTargetUrl));
	// Use first segment of scene for resource
	// 'workspaces/sample-workspace'
	TArray<FString> Split;
	SceneId.ParseIntoArray(Split, TEXT("/"));
	if (Split.Num() >= 2)
	{
		Options.Resource = TCHAR_TO_UTF8(*FString(Split[0] + "/" + Split[1]));
	}

	Options.SceneName = TCHAR_TO_UTF8(*SceneId);
	Options.Base64 = TCHAR_TO_UTF8(*Auth.Base64Signature);
	Options.ApiKey = TCHAR_TO_UTF8(*Auth.ApiKey);
	Options.ApiSecret = TCHAR_TO_UTF8(*Auth.ApiSecret);
	Options.ProjectName = TCHAR_TO_UTF8(!PlayerProfile.ProjectName.IsEmpty() ? *PlayerProfile.ProjectName : FApp::GetProjectName());

	ConvertPlayerProfile(PlayerProfile, Options.UserConfig);
	ConvertCapabilities(CapabilitySet, Options.Capabilities);

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
	
	AudioSender->Initialize(true);
}

void UInworldClient::StopSession()
{
	NO_CLIENT_RETURN(void())

	AudioSender->Terminate();
	Inworld::GetClient()->StopClient();
}

void UInworldClient::PauseSession()
{
	NO_CLIENT_RETURN(void())

	Inworld::GetClient()->PauseClient();
}

void UInworldClient::ResumeSession()
{
	NO_CLIENT_RETURN(void())

	Inworld::GetClient()->ResumeClient();
}

void UInworldClient::SaveSession(FOnInworldSessionSavedCallback Callback)
{
	NO_CLIENT_RETURN(void())

	Inworld::GetClient()->SaveSessionStateAsync([Callback](std::string Data, bool bSuccess)
		{
			FInworldSave Save;
			if (bSuccess)
			{
				Save.Data.SetNumUninitialized(Data.size());
				FMemory::Memcpy(Save.Data.GetData(), (uint8*)Data.data(), Save.Data.Num());
			}
			AsyncTask(ENamedThreads::GameThread, [Callback, Save, bSuccess]()
				{
					if (!bSuccess)
					{
						UE_LOG(LogInworldAIClient, Error, TEXT("Failed to save."));
					}
					Callback.ExecuteIfBound(Save, bSuccess);
				}
			);
		});
}

void UInworldClient::LoadCharacters(const TArray<FString>& Ids)
{
	NO_CLIENT_RETURN(void())
	EMPTY_ARG_RETURN(Ids, void())

	Inworld::GetClient()->LoadCharacters(ToStd(Ids));
}

void UInworldClient::UnloadCharacters(const TArray<FString>& Ids)
{
	NO_CLIENT_RETURN(void())
	EMPTY_ARG_RETURN(Ids, void())

	Inworld::GetClient()->UnloadCharacters(ToStd(Ids));
}

void UInworldClient::LoadSavedState(const FInworldSave& Save)
{
	NO_CLIENT_RETURN(void())

	std::string Data((char*)Save.Data.GetData(), Save.Data.Num());
	Inworld::GetClient()->LoadSavedState(Data);
}

void UInworldClient::LoadCapabilities(const FInworldCapabilitySet& CapabilitySet)
{
	NO_CLIENT_RETURN(void())

	Inworld::Capabilities Capabilities;
	ConvertCapabilities(CapabilitySet, Capabilities);
	Inworld::GetClient()->LoadCapabilities(Capabilities);
}

void UInworldClient::LoadPlayerProfile(const FInworldPlayerProfile& PlayerProfile)
{
	NO_CLIENT_RETURN(void())

	Inworld::UserConfiguration UserConfig;
	ConvertPlayerProfile(PlayerProfile, UserConfig);
	Inworld::GetClient()->LoadUserConfiguration(UserConfig);
}

FString UInworldClient::UpdateConversation(const FString& ConversationId, const TArray<FString>& AgentIds, bool bIncludePlayer)
{
	NO_CLIENT_RETURN({})

	if (AgentIds.Num() == 0)
	{
		return {};
	}

	auto Packet = Inworld::GetClient()->UpdateConversation(ToStd(AgentIds), TCHAR_TO_UTF8(*ConversationId), bIncludePlayer);
	return UTF8_TO_TCHAR(Packet->_Routing._ConversationId.c_str());
}

EInworldConnectionState UInworldClient::GetConnectionState() const
{
	NO_CLIENT_RETURN(EInworldConnectionState::Idle)

	return static_cast<EInworldConnectionState>(Inworld::GetClient()->GetConnectionState());
}

void UInworldClient::GetConnectionError(FString& OutErrorMessage, int32& OutErrorCode) const
{
	NO_CLIENT_RETURN(void())

	std::string OutError;
	Inworld::GetClient()->GetConnectionError(OutError, OutErrorCode);
	OutErrorMessage = UTF8_TO_TCHAR(OutError.c_str());
}

FString UInworldClient::GetSessionId() const
{
	NO_CLIENT_RETURN({})

	return UTF8_TO_TCHAR(Inworld::GetClient()->GetSessionInfo().SessionId.c_str());
}

FInworldWrappedPacket UInworldClient::SendTextMessage(const FString& AgentId, const FString& Text)
{
	NO_CLIENT_RETURN({})
	EMPTY_ARG_RETURN(AgentId, {})
	EMPTY_ARG_RETURN(Text, {})

	auto Packet = Inworld::GetClient()->SendTextMessage(TCHAR_TO_UTF8(*AgentId), TCHAR_TO_UTF8(*Text));
	InworldPacketTranslator PacketTranslator;
	Packet->Accept(PacketTranslator);
	return PacketTranslator.GetPacket();
}

FInworldWrappedPacket UInworldClient::SendTextMessageToConversation(const FString& ConversationId, const FString& Text)
{
	NO_CLIENT_RETURN({})
	EMPTY_ARG_RETURN(ConversationId, {})
	EMPTY_ARG_RETURN(Text, {})

	auto Packet = Inworld::GetClient()->SendTextMessageToConversation(TCHAR_TO_UTF8(*ConversationId), TCHAR_TO_UTF8(*Text));
	InworldPacketTranslator PacketTranslator;
	Packet->Accept(PacketTranslator);
	return PacketTranslator.GetPacket();
}

void UInworldClient::SendSoundMessage(const FString& AgentId, const TArray<uint8>& InputData, const TArray<uint8>& OutputData)
{
	NO_CLIENT_RETURN(void())
	EMPTY_ARG_RETURN(AgentId, void())
	EMPTY_ARG_RETURN(InputData, void())

	std::vector<int16> inputdata((int16*)InputData.GetData(), ((int16*)InputData.GetData()) + (InputData.Num() / 2));
	if (OutputData.Num() > 0)
	{
		AudioSender->SendSoundMessage(TCHAR_TO_UTF8(*AgentId), inputdata);
	}
	else
	{
		std::vector<int16> outputdata((int16*)OutputData.GetData(), ((int16*)OutputData.GetData()) + (OutputData.Num() / 2));
		AudioSender->SendSoundMessageWithAEC(TCHAR_TO_UTF8(*AgentId), inputdata, outputdata);
	}
}

void UInworldClient::SendSoundMessageToConversation(const FString& ConversationId, const TArray<uint8>& InputData, const TArray<uint8>& OutputData)
{
	NO_CLIENT_RETURN(void())
	EMPTY_ARG_RETURN(ConversationId, void())
	EMPTY_ARG_RETURN(InputData, void())

	std::vector<int16> inputdata((int16*)InputData.GetData(), ((int16*)InputData.GetData()) + (InputData.Num() / 2));
	if (OutputData.Num() == 0)
	{
		AudioSender->SendSoundMessageToConversation(TCHAR_TO_UTF8(*ConversationId), inputdata);
	}
	else
	{
		std::vector<int16> outputdata((int16*)OutputData.GetData(), ((int16*)OutputData.GetData()) + (OutputData.Num() / 2));
		AudioSender->SendSoundMessageWithAECToConversation(TCHAR_TO_UTF8(*ConversationId), inputdata, outputdata);
	}
}

void UInworldClient::SendAudioSessionStart(const FString& AgentId, EInworldMicrophoneMode MicrophoneMode/* = EInworldMicrophoneMode::OPEN_MIC*/)
{
	NO_CLIENT_RETURN(void())
	EMPTY_ARG_RETURN(AgentId, void())

	AudioSender->StartAudioSession(TCHAR_TO_UTF8(*AgentId), MicrophoneMode);
}

void UInworldClient::SendAudioSessionStartToConversation(const FString& ConversationId, EInworldMicrophoneMode MicrophoneMode/* = EInworldMicrophoneMode::OPEN_MIC*/)
{
	NO_CLIENT_RETURN(void())
	EMPTY_ARG_RETURN(ConversationId, void())

	AudioSender->StartAudioSessionInConversation(TCHAR_TO_UTF8(*ConversationId), MicrophoneMode);
}

void UInworldClient::SendAudioSessionStop(const FString& AgentId)
{
	NO_CLIENT_RETURN(void())
	EMPTY_ARG_RETURN(AgentId, void())

	AudioSender->StopAudioSession(TCHAR_TO_UTF8(*AgentId));
}

void UInworldClient::SendAudioSessionStopToConversation(const FString& ConversationId)
{
	NO_CLIENT_RETURN(void())
	EMPTY_ARG_RETURN(ConversationId, void())

	Inworld::VAD_Terminate();

	AudioSender->StopAudioSessionInConversation(TCHAR_TO_UTF8(*ConversationId));
}

void UInworldClient::SendTrigger(const FString& AgentId, const FString& Name, const TMap<FString, FString>& Params)
{
	NO_CLIENT_RETURN(void())
	EMPTY_ARG_RETURN(AgentId, void())
	EMPTY_ARG_RETURN(Name, void())

	Inworld::GetClient()->SendCustomEvent(TCHAR_TO_UTF8(*AgentId), TCHAR_TO_UTF8(*Name), ToStd(Params));
}

void UInworldClient::SendTriggerToConversation(const FString& ConversationId, const FString& Name, const TMap<FString, FString>& Params)
{
	NO_CLIENT_RETURN(void())
	EMPTY_ARG_RETURN(ConversationId, void())
	EMPTY_ARG_RETURN(Name, void())

	Inworld::GetClient()->SendCustomEventToConversation(TCHAR_TO_UTF8(*ConversationId), TCHAR_TO_UTF8(*Name), ToStd(Params));
}

void UInworldClient::SendChangeSceneEvent(const FString& SceneName)
{
	NO_CLIENT_RETURN(void())
	EMPTY_ARG_RETURN(SceneName, void())

	Inworld::GetClient()->LoadScene(TCHAR_TO_UTF8(*SceneName));
}

void UInworldClient::SendNarrationEvent(const FString& AgentId, const FString& Content)
{
	NO_CLIENT_RETURN(void())
	EMPTY_ARG_RETURN(AgentId, void())
	EMPTY_ARG_RETURN(Content, void())

	Inworld::GetClient()->SendNarrationEvent(TCHAR_TO_UTF8(*AgentId), TCHAR_TO_UTF8(*Content));
}

void UInworldClient::CancelResponse(const FString& AgentId, const FString& InteractionId, const TArray<FString>& UtteranceIds)
{
	NO_CLIENT_RETURN(void())
	EMPTY_ARG_RETURN(AgentId, void())
	EMPTY_ARG_RETURN(InteractionId, void())
	EMPTY_ARG_RETURN(UtteranceIds, void())

	std::vector<std::string> utteranceIds;
	utteranceIds.reserve(UtteranceIds.Num());
	for (auto& Id : UtteranceIds)
	{
		utteranceIds.push_back(TCHAR_TO_UTF8(*Id));
	}

	Inworld::GetClient()->CancelResponse(TCHAR_TO_UTF8(*AgentId), TCHAR_TO_UTF8(*InteractionId), utteranceIds);
}

#if !UE_BUILD_SHIPPING
void UInworldClient::OnCVarsChanged()
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

#undef EMPTY_ARG_RETURN
#undef NO_CLIENT_RETURN
