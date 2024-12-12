/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */
#ifdef INWORLD_WITH_NDK

#include "InworldClientNDK.h"
#include "InworldAIClientModule.h"
#include "InworldAIClientSettings.h"

#include "InworldPacketTranslator.h"

THIRD_PARTY_INCLUDES_START
#include "Packets.h"
#include "Client.h"
#include "Utils/Log.h"
#include "Utils/Utils.h"
THIRD_PARTY_INCLUDES_END

#include "Misc/Paths.h"
#include "Misc/App.h"
#include <Interfaces/IPluginManager.h>
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

InworldClientNDK::FOnAudioDumperCVarChanged InworldClientNDK::OnAudioDumperCVarChanged;

FAutoConsoleVariableSink InworldClientNDK::CVarSink(FConsoleCommandDelegate::CreateStatic(&InworldClientNDK::OnCVarsChanged));
#endif

std::vector<std::string> ToStd(const TArray<FString>& Array)
{
	std::vector<std::string> StdVec;
	for (auto& Entry : Array)
	{
		StdVec.emplace_back(TCHAR_TO_UTF8(*Entry));
	}
	return StdVec;
}

std::pair<std::string, std::string> ToStd(const TPair<FString, FString>& Pair)
{
	return std::make_pair<std::string, std::string>(TCHAR_TO_UTF8(*Pair.Key), TCHAR_TO_UTF8(*Pair.Value));
}

std::unordered_map<std::string, std::string> ToStd(const TMap<FString, FString>& Map)
{
	std::unordered_map<std::string, std::string> StdMap;
	for (const TPair<FString, FString>& Entry : Map)
	{
		StdMap.insert(ToStd(Entry));
	}
	return StdMap;
}

template<typename T>
void DataArrayToVec(const TArray<T>& ArrData, std::vector<T>& VecData)
{
	VecData.resize(ArrData.Num() * sizeof(T));
	FMemory::Memcpy((void*)VecData.data(), (void*)ArrData.GetData(), VecData.size());
}

template<typename T>
void VecToDataArray(const std::vector<T>& VecData, TArray<T>& ArrData)
{
	ArrData.SetNumUninitialized(VecData.size() * sizeof(T));
	FMemory::Memcpy((void*)ArrData.GetData(), (void*)VecData.data(), ArrData.Num());
}

#ifdef INWORLD_WITH_NDK
static TArray<uint8> HmacSha256(const TArray<uint8>& Data, const TArray<uint8>& Key)
{
	std::vector<uint8> data;
	std::vector<uint8> key;
	DataArrayToVec(Data, data);
	DataArrayToVec(Key, key);

	std::vector<uint8> result(32);
	Inworld::Utils::HmacSha256(data, key, result);
	TArray<uint8> Result;
	VecToDataArray(result, Result);
	return Result;
}
#endif

static FString ToHex(const TArray<uint8>& Data)
{
	std::string result(Data.Num() * 2, '0');
	for (int32 i = 0; i < Data.Num(); i++)
	{
		FCStringAnsi::Sprintf((char*)(result.data()) + (i * 2), "%02x", Data[i]);
	}

	return FString(UTF8_TO_TCHAR(result.c_str()));
}

static FString GetResourceFromSettings(const FString& WorkspaceOverride)
{
	const UInworldAIClientSettings* InworldAIClientSettings = GetDefault<UInworldAIClientSettings>();

	if (!WorkspaceOverride.IsEmpty())
	{
		return FString("workspaces/") + WorkspaceOverride;
	}
	else if (!InworldAIClientSettings->Workspace.IsEmpty())
	{
		return FString("workspaces/") + InworldAIClientSettings->Workspace;
	}
	return {};
}

template<class T, class U>
static void ConvertCapabilities(const T& Capabilities, U& OutCapabilities)
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
	OutCapabilities.MultiAgent = Capabilities.MultiAgent;
	OutCapabilities.Audio2Face = Capabilities.Audio2Face;
	OutCapabilities.MultiModalActionPlanning = Capabilities.MultiModalActionPlanning;
	OutCapabilities.Logs = Capabilities.Logs;
	OutCapabilities.LogsWarning = Capabilities.LogsWarning;
	OutCapabilities.LogsInfo = Capabilities.LogsInfo;
	OutCapabilities.LogsDebug = Capabilities.LogsDebug;
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
		return FString{};
	}

#ifdef INWORLD_WITH_NDK

	UE_LOG(LogInworldAIClient, Log, TEXT("Device Id: %s"), *Id);

	std::string SId = TCHAR_TO_UTF8(*Id);
	TArray<uint8> Data;
	Data.SetNumZeroed(SId.size());
	FMemory::Memcpy(Data.GetData(), SId.data(), SId.size());

	Data = HmacSha256(Data, Data);

	return ToHex(Data);
#else
	return FString{};
#endif
}

static void ConvertSceneToSceneId(const FInworldScene& Scene, const FString& Resource, std::string& SceneId)
{
	const FString SceneName = [Scene]() -> FString
		{
			TArray<FString> Split;
			Scene.Name.ParseIntoArray(Split, TEXT("/"));
			if (Split.Num() == 4)
			{
				return Split[2] + "/" + Split[3];
			}
			return [Type = Scene.Type]() -> FString
				{
					switch (Type)
					{
					case EInworldSceneType::SCENE:
						return FString("scenes/");
					case EInworldSceneType::CHARACTER:
						return FString("characters/");
					default:
						UE_LOG(LogInworldAIClient, Warning, TEXT("Unknown scene type! Defaulting to regular scene."));
						return FString("scenes/");
					}
				}
			() + Scene.Name;
		}
	();

	SceneId = TCHAR_TO_UTF8(*FString(Resource + ("/") + SceneName));
}

static void ConvertSceneIdToScene(const std::string& SceneId, FInworldScene& Scene)
{
	TArray<FString> Split;
	FString{ UTF8_TO_TCHAR(SceneId.c_str()) }.ParseIntoArray(Split, TEXT("/"));
	if (Split.Num() == 4)
	{
		if (Split[2] == TEXT("characters"))
		{
			Scene.Type = EInworldSceneType::CHARACTER;
		}
		else if (Split[2] == TEXT("scenes"))
		{
			Scene.Type = EInworldSceneType::SCENE;
		}
		Scene.Name = UTF8_TO_TCHAR(SceneId.c_str());
	}
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

static Inworld::ClientOptions CreateClientOptions(const FInworldScene& Scene, const FInworldPlayerProfile& PlayerProfile, const FInworldCapabilitySet& CapabilitySet, const TMap<FString, FString>& Metadata, const FString& WorkspaceOverride, const FInworldAuth& AuthOverride)
{
	Inworld::ClientOptions Options;

	const UInworldAIClientSettings* InworldAIClientSettings = GetDefault<UInworldAIClientSettings>();
	Options.ServerUrl = TCHAR_TO_UTF8(*InworldAIClientSettings->Environment.TargetUrl);
	Options.Resource = [&]() -> std::string
		{
			if (!WorkspaceOverride.IsEmpty())
			{
				return TCHAR_TO_UTF8(*(FString("workspaces/") + WorkspaceOverride));
			}
			else if (!InworldAIClientSettings->Workspace.IsEmpty())
			{
				return TCHAR_TO_UTF8(*(FString("workspaces/") + InworldAIClientSettings->Workspace));
			}
			else
			{
				// Use first segment of scene for resource
				// 'workspaces/sample-workspace'
				TArray<FString> Split;
				Scene.Name.ParseIntoArray(Split, TEXT("/"));
				if (Split.Num() == 4)
				{
					return TCHAR_TO_UTF8(*FString(Split[0] + "/" + Split[1]));
				}
			}
			return {};
		}();

	const FInworldAuth DefaultAuth = InworldAIClientSettings->Auth;
	Options.Base64 = TCHAR_TO_UTF8(AuthOverride.Base64Signature.IsEmpty() ? *DefaultAuth.Base64Signature : *AuthOverride.Base64Signature);
	Options.ApiKey = TCHAR_TO_UTF8(AuthOverride.ApiKey.IsEmpty() ? *DefaultAuth.ApiKey : *AuthOverride.ApiKey);
	Options.ApiSecret = TCHAR_TO_UTF8(AuthOverride.ApiSecret.IsEmpty() ? *DefaultAuth.ApiSecret : *AuthOverride.ApiSecret);
	Options.ProjectName = TCHAR_TO_UTF8(!PlayerProfile.ProjectName.IsEmpty() ? *PlayerProfile.ProjectName : FApp::GetProjectName());

	ConvertPlayerProfile(PlayerProfile, Options.UserConfig);
	ConvertCapabilities(CapabilitySet, Options.Capabilities);

	for (const TPair<FString, FString>& Entry : Metadata)
	{
		Options.Metadata.emplace_back(ToStd(Entry));
	}

	return Options;
}

InworldClientNDK::InworldClientNDK()
{
	// Ensure dependencies are loaded
	FInworldAIClientModule::Get();

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

	NDKClient = Inworld::CreateClient();
	NDKClient->InitClientAsync(Sdk,
		[this](Inworld::Client::ConnectionState ConnectionState)
		{
			OnConnectionStateChanged().Broadcast(static_cast<EInworldConnectionState>(ConnectionState));
		},
		[this](std::shared_ptr<Inworld::Packet> Packet)
		{
			InworldPacketTranslator PacketTranslator;
			Packet->Accept(PacketTranslator);
			TSharedPtr<FInworldPacket> ReceivedPacket = PacketTranslator.GetPacket();
			if (ReceivedPacket.IsValid())
			{
				OnPacketReceived().Broadcast(ReceivedPacket);
			}
		}
	);

	NDKClient->SetPerceivedLatencyTrackerCallback([this](const std::string& InteractionId, uint32_t LatencyMs)
		{
			OnPerceivedLatency().Broadcast(UTF8_TO_TCHAR(InteractionId.c_str()), LatencyMs);
		}
	);

#if !UE_BUILD_SHIPPING
	auto OnAudioDumperCVarChangedCallback = [this](bool bEnable, FString Path)
		{
			if (NDKClient->GetConnectionState() == Inworld::Client::ConnectionState::Idle)
			{
				return;
			}

			const std::string DumpPath = TCHAR_TO_UTF8(*CVarSoundDumpPath.GetValueOnGameThread());
			if (bEnable)
			{
				NDKClient->EnableAudioDump(DumpPath);
			}
			else
			{
				NDKClient->DisableAudioDump();
			}
		};
	OnAudioDumperCVarChangedHandle = OnAudioDumperCVarChanged.AddLambda(OnAudioDumperCVarChangedCallback);
	OnAudioDumperCVarChangedCallback(CVarEnableSoundDump.GetValueOnGameThread(), CVarSoundDumpPath.GetValueOnGameThread());
#endif
}

InworldClientNDK::~InworldClientNDK()
{
	Inworld::DestroyClient(std::move(NDKClient));
#if !UE_BUILD_SHIPPING
	OnAudioDumperCVarChanged.Remove(OnAudioDumperCVarChangedHandle);
#endif
}

void InworldClientNDK::StartSessionFromScene(const FInworldScene& Scene, const FInworldPlayerProfile& PlayerProfile, const FInworldCapabilitySet& CapabilitySet, const TMap<FString, FString>& Metadata, const FString& WorkspaceOverride, const FInworldAuth& AuthOverride)
{
	Inworld::ClientOptions Options = CreateClientOptions(Scene, PlayerProfile, CapabilitySet, Metadata, WorkspaceOverride, AuthOverride);
	NDKClient->SetOptions(Options);

	std::string SceneId;
	ConvertSceneToSceneId(Scene, UTF8_TO_TCHAR(Options.Resource.c_str()), SceneId);
	NDKClient->StartClientFromSceneId(SceneId);
}

void InworldClientNDK::StartSessionFromSave(const FInworldSave& Save, const FInworldPlayerProfile& PlayerProfile, const FInworldCapabilitySet& CapabilitySet, const TMap<FString, FString>& Metadata, const FString& WorkspaceOverride, const FInworldAuth& AuthOverride)
{
	Inworld::ClientOptions Options = CreateClientOptions(Save.Scene, PlayerProfile, CapabilitySet, Metadata, WorkspaceOverride, AuthOverride);
	NDKClient->SetOptions(Options);

	Inworld::SessionSave SessionSave;
	ConvertSceneToSceneId(Save.Scene, UTF8_TO_TCHAR(Options.Resource.c_str()), SessionSave.SceneId);
	SessionSave.State.resize(Save.State.Num());
	FMemory::Memcpy((uint8*)SessionSave.State.data(), (uint8*)Save.State.GetData(), SessionSave.State.size());
	NDKClient->StartClientFromSave(SessionSave);
}

void InworldClientNDK::StartSessionFromToken(const FInworldToken& Token, const FInworldPlayerProfile& PlayerProfile, const FInworldCapabilitySet& CapabilitySet, const TMap<FString, FString>& Metadata, const FString& WorkspaceOverride, const FInworldAuth& AuthOverride)
{
	Inworld::ClientOptions Options = CreateClientOptions({}, PlayerProfile, CapabilitySet, Metadata, WorkspaceOverride, AuthOverride);
	NDKClient->SetOptions(Options);

	Inworld::SessionToken SessionToken;
	SessionToken.Token = TCHAR_TO_UTF8(*Token.Token);
	SessionToken.ExpirationTime = Token.ExpirationTime;
	SessionToken.SessionId = TCHAR_TO_UTF8(*Token.SessionId);
	NDKClient->StartClientFromToken(SessionToken);
}

void InworldClientNDK::StopSession()
{
	NDKClient->StopClient();
}

void InworldClientNDK::PauseSession()
{
	NDKClient->PauseClient();
}

void InworldClientNDK::ResumeSession()
{
	NDKClient->ResumeClient();
}

FInworldToken InworldClientNDK::GetSessionToken() const
{
	FInworldToken Token;
	Inworld::SessionToken SessionToken = NDKClient->GetSessionToken();
	Token.Token = UTF8_TO_TCHAR(SessionToken.Token.c_str());
	Token.ExpirationTime = SessionToken.ExpirationTime;
	Token.SessionId = UTF8_TO_TCHAR(SessionToken.SessionId.c_str());
	return Token;
}

void InworldClientNDK::LoadPlayerProfile(const FInworldPlayerProfile& PlayerProfile)
{
	Inworld::UserConfiguration UserConfig;
	ConvertPlayerProfile(PlayerProfile, UserConfig);
	NDKClient->LoadUserConfiguration(UserConfig);
}

FInworldCapabilitySet InworldClientNDK::GetCapabilities() const
{
	FInworldCapabilitySet CapabilitySet;
	ConvertCapabilities(NDKClient->GetOptions().Capabilities, CapabilitySet);
	return CapabilitySet;
}

void InworldClientNDK::LoadCapabilities(const FInworldCapabilitySet& CapabilitySet)
{
	Inworld::Capabilities Capabilities;
	ConvertCapabilities(CapabilitySet, Capabilities);
	NDKClient->LoadCapabilities(Capabilities);
}

void InworldClientNDK::SaveSession(FOnInworldSessionSavedCallback Callback)
{
	NDKClient->SaveSessionStateAsync([Callback](const Inworld::SessionSave& SessionSave, bool bSuccess)
		{
			FInworldSave Save;
			if (bSuccess)
			{
				ConvertSceneIdToScene(SessionSave.SceneId, Save.Scene);
				Save.State.SetNumUninitialized(SessionSave.State.size());
				FMemory::Memcpy(Save.State.GetData(), (uint8*)SessionSave.State.data(), Save.State.Num());
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

void InworldClientNDK::SendInteractionFeedback(const FString& InteractionId, bool bIsLike, const FString& Message)
{
	Inworld::InteractionFeedback InteractionFeedback;
	InteractionFeedback.bIsLike = true;
	InteractionFeedback.comment = TCHAR_TO_UTF8(*Message);
	std::string interaction = TCHAR_TO_UTF8(*InteractionId);
	NDKClient->SendFeedbackAsync(interaction, InteractionFeedback);
}

void InworldClientNDK::LoadCharacter(const FString& Id)
{
	LoadCharacters({ Id });
}

void InworldClientNDK::LoadCharacters(const TArray<FString>& Ids)
{
	NDKClient->LoadCharacters(ToStd(Ids));
}

void InworldClientNDK::UnloadCharacter(const FString& Id)
{
	UnloadCharacters({ Id });
}

void InworldClientNDK::UnloadCharacters(const TArray<FString>& Ids)
{
	NDKClient->UnloadCharacters(ToStd(Ids));
}

FString InworldClientNDK::UpdateConversation(const FString& ConversationId, const TArray<FString>& AgentIds, bool bIncludePlayer)
{
	auto Packet = NDKClient->UpdateConversation(ToStd(AgentIds), TCHAR_TO_UTF8(*ConversationId), bIncludePlayer);
	return UTF8_TO_TCHAR(Packet->_Routing._ConversationId.c_str());
}

FInworldWrappedPacket InworldClientNDK::SendTextMessage(const FString& AgentId, const FString& Text)
{
	auto Packet = NDKClient->SendTextMessage(TCHAR_TO_UTF8(*AgentId), TCHAR_TO_UTF8(*Text));
	InworldPacketTranslator PacketTranslator;
	Packet->Accept(PacketTranslator);
	return PacketTranslator.GetPacket();
}

FInworldWrappedPacket InworldClientNDK::SendTextMessageToConversation(const FString& ConversationId, const FString& Text)
{
	auto Packet = NDKClient->SendTextMessageToConversation(TCHAR_TO_UTF8(*ConversationId), TCHAR_TO_UTF8(*Text));
	InworldPacketTranslator PacketTranslator;
	Packet->Accept(PacketTranslator);
	return PacketTranslator.GetPacket();
}

void InworldClientNDK::InitSpeechProcessor(EInworldPlayerSpeechMode Mode, const FInworldPlayerSpeechOptions& SpeechOptions)
{
	const FString Path = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("InworldAI"))->GetBaseDir(), TEXT("Source/ThirdParty/InworldAINDKLibrary/resource/silero_vad_10_27_2022.onnx"));

	Inworld::ClientSpeechOptions_VAD Options_VAD;
	Options_VAD.VADModelPath = TCHAR_TO_UTF8(*Path);
	Options_VAD.VADProbThreshhold = SpeechOptions.VADProbThreshhold;
	Options_VAD.VADBufferChunksNum = SpeechOptions.VADBufferChunksNum;
	Options_VAD.VADSilenceChunksNum = SpeechOptions.VADSilenceChunksNum;

	switch (Mode)
	{
	case EInworldPlayerSpeechMode::DEFAULT:
		NDKClient->InitSpeechProcessor(Inworld::ClientSpeechOptions_Default{});
		break;
	case EInworldPlayerSpeechMode::VAD_DETECT_ONLY:
		NDKClient->InitSpeechProcessor(Inworld::ClientSpeechOptions_VAD_DetectOnly{ Options_VAD });
		break;
	case EInworldPlayerSpeechMode::VAD_DETECT_AND_FILTER:
		NDKClient->InitSpeechProcessor(Inworld::ClientSpeechOptions_VAD_DetectAndFilterAudio{ Options_VAD });
		break;
	}
}

void InworldClientNDK::DestroySpeechProcessor()
{
	NDKClient->DestroySpeechProcessor();
}

void InworldClientNDK::SendSoundMessage(const FString& AgentId, const TArray<uint8>& InputData, const TArray<uint8>& OutputData)
{
	if (OutputData.Num() > 0)
	{
		std::string inputdata((char*)InputData.GetData(), InputData.Num());
		NDKClient->SendSoundMessage(TCHAR_TO_UTF8(*AgentId), inputdata);
	}
	else
	{
		std::vector<int16> inputdata((int16*)InputData.GetData(), ((int16*)InputData.GetData()) + (InputData.Num() / 2));
		std::vector<int16> outputdata((int16*)OutputData.GetData(), ((int16*)OutputData.GetData()) + (OutputData.Num() / 2));
		NDKClient->SendSoundMessageWithAEC(TCHAR_TO_UTF8(*AgentId), inputdata, outputdata);
	}
}

void InworldClientNDK::SendSoundMessageToConversation(const FString& ConversationId, const TArray<uint8>& InputData, const TArray<uint8>& OutputData)
{
	if (OutputData.Num() == 0)
	{
		std::string inputdata((char*)InputData.GetData(), InputData.Num());
		NDKClient->SendSoundMessageToConversation(TCHAR_TO_UTF8(*ConversationId), inputdata);
	}
	else
	{
		std::vector<int16> inputdata((int16*)InputData.GetData(), ((int16*)InputData.GetData()) + (InputData.Num() / 2));
		std::vector<int16> outputdata((int16*)OutputData.GetData(), ((int16*)OutputData.GetData()) + (OutputData.Num() / 2));
		NDKClient->SendSoundMessageWithAECToConversation(TCHAR_TO_UTF8(*ConversationId), inputdata, outputdata);
	}
}

void InworldClientNDK::SendAudioSessionStart(const FString& AgentId, FInworldAudioSessionOptions SessionOptions)
{
	Inworld::AudioSessionStartPayload AudioPayload;
	AudioPayload.MicMode = static_cast<Inworld::AudioSessionStartPayload::MicrophoneMode>(SessionOptions.MicrophoneMode);
	AudioPayload.UndMode = static_cast<Inworld::AudioSessionStartPayload::UnderstandingMode>(SessionOptions.UnderstandingMode);
	NDKClient->StartAudioSession(TCHAR_TO_UTF8(*AgentId), AudioPayload);
}

void InworldClientNDK::SendAudioSessionStartToConversation(const FString& ConversationId, FInworldAudioSessionOptions SessionOptions)
{
	Inworld::AudioSessionStartPayload AudioPayload;
	AudioPayload.MicMode = static_cast<Inworld::AudioSessionStartPayload::MicrophoneMode>(SessionOptions.MicrophoneMode);
	AudioPayload.UndMode = static_cast<Inworld::AudioSessionStartPayload::UnderstandingMode>(SessionOptions.UnderstandingMode);
	NDKClient->StartAudioSessionInConversation(TCHAR_TO_UTF8(*ConversationId), AudioPayload);
}

void InworldClientNDK::SendAudioSessionStop(const FString& AgentId)
{
	NDKClient->StopAudioSession(TCHAR_TO_UTF8(*AgentId));
}

void InworldClientNDK::SendAudioSessionStopToConversation(const FString& ConversationId)
{
	NDKClient->StopAudioSessionInConversation(TCHAR_TO_UTF8(*ConversationId));
}

void InworldClientNDK::SendNarrationEvent(const FString& AgentId, const FString& Content)
{
	NDKClient->SendNarrationEvent(TCHAR_TO_UTF8(*AgentId), TCHAR_TO_UTF8(*Content));
}

void InworldClientNDK::SendTrigger(const FString& AgentId, const FString& Name, const TMap<FString, FString>& Params)
{
	NDKClient->SendCustomEvent(TCHAR_TO_UTF8(*AgentId), TCHAR_TO_UTF8(*Name), ToStd(Params));
}

void InworldClientNDK::SendTriggerToConversation(const FString& ConversationId, const FString& Name, const TMap<FString, FString>& Params)
{
	NDKClient->SendCustomEventToConversation(TCHAR_TO_UTF8(*ConversationId), TCHAR_TO_UTF8(*Name), ToStd(Params));
}

void InworldClientNDK::SendChangeSceneEvent(const FString& SceneName)
{
	NDKClient->LoadScene(TCHAR_TO_UTF8(*SceneName));
}

void InworldClientNDK::CancelResponse(const FString& AgentId, const FString& InteractionId, const TArray<FString>& UtteranceIds)
{
	std::vector<std::string> utteranceIds;
	utteranceIds.reserve(UtteranceIds.Num());
	for (auto& Id : UtteranceIds)
	{
		utteranceIds.push_back(TCHAR_TO_UTF8(*Id));
	}

	NDKClient->CancelResponse(TCHAR_TO_UTF8(*AgentId), TCHAR_TO_UTF8(*InteractionId), utteranceIds);
}

void InworldClientNDK::CreateOrUpdateItems(const TArray<FInworldEntityItem>& Items, const TArray<FString>& AddToEntities)
{
	std::vector<Inworld::CreateOrUpdateItemsOperationEvent::EntityItem> items;
	items.reserve(Items.Num());
	for (const FInworldEntityItem& Item : Items)
	{
		Inworld::CreateOrUpdateItemsOperationEvent::EntityItem& item = items.emplace_back();
		item.Id = TCHAR_TO_UTF8(*Item.Id);
		item.Description = TCHAR_TO_UTF8(*Item.Description);
		item.DisplayName = TCHAR_TO_UTF8(*Item.DisplayName);
		item.Properties = ToStd(Item.Properties);
	}

	NDKClient->CreateOrUpdateItems(items, ToStd(AddToEntities));
}

void InworldClientNDK::RemoveItems(const TArray<FString>& ItemIds)
{
	NDKClient->RemoveItems(ToStd(ItemIds));
}

void InworldClientNDK::AddItemsInEntities(const TArray<FString>& ItemIds, const TArray<FString>& EntityNames)
{
	NDKClient->AddItemsInEntities(ToStd(ItemIds), ToStd(EntityNames));
}

void InworldClientNDK::RemoveItemsInEntities(const TArray<FString>& ItemIds, const TArray<FString>& EntityNames)
{
	NDKClient->RemoveItemsInEntities(ToStd(ItemIds), ToStd(EntityNames));
}

void InworldClientNDK::ReplaceItemsInEntities(const TArray<FString>& ItemIds, const TArray<FString>& EntityNames)
{
	NDKClient->ReplaceItemsInEntities(ToStd(ItemIds), ToStd(EntityNames));
}

EInworldConnectionState InworldClientNDK::GetConnectionState() const
{
	return static_cast<EInworldConnectionState>(NDKClient->GetConnectionState());
}

void InworldClientNDK::GetConnectionError(FString& OutErrorMessage, int32& OutErrorCode, FInworldConnectionErrorDetails& OutErrorDetails) const
{
	std::string ErrorMessage;
	int32_t ErrorCode;
	Inworld::ErrorDetails ErrorDetails;

	NDKClient->GetConnectionError(ErrorMessage, ErrorCode, ErrorDetails);

	OutErrorMessage = UTF8_TO_TCHAR(ErrorMessage.c_str());
	OutErrorCode = ErrorCode;
	OutErrorDetails.ConnectionErrorType = static_cast<EInworldConnectionErrorType>(ErrorDetails.Error);
	OutErrorDetails.ReconnectionType = static_cast<EInworldReconnectionType>(ErrorDetails.Reconnect);
	OutErrorDetails.ReconnectTime = ErrorDetails.ReconnectTime;
	OutErrorDetails.MaxRetries = ErrorDetails.MaxRetries;
}

#if !UE_BUILD_SHIPPING
void InworldClientNDK::OnCVarsChanged()
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

#endif
