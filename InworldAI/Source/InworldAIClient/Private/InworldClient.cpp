/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldClient.h"
#include "InworldAIClientModule.h"
#include "InworldAIClientSettings.h"
#include "InworldMacros.h"
#include "CoreMinimal.h"
#include "GenericPlatform/GenericPlatformMisc.h"

#include "SocketSubsystem.h"
#include "IPAddress.h"

#include "InworldAINDKModule.h"
#include "InworldPacketTranslator.h"

THIRD_PARTY_INCLUDES_START
#include "Packets.h"
#include "Client.h"
#include "Utils/Log.h"
#include "Utils/Utils.h"
THIRD_PARTY_INCLUDES_END

#include "Async/Async.h"
#include "Async/TaskGraphInterfaces.h"

#include <Interfaces/IPluginManager.h>

#include "Misc/Paths.h"
#include "Misc/App.h"

#include <string>
#include <memory>

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

class NDKClientImpl : public NDKClient
{
public:
	NDKClientImpl()
	{
		_Client = Inworld::CreateClient();
	}
	virtual ~NDKClientImpl()
	{
		Inworld::DestroyClient(std::move(_Client));
	}

	virtual Inworld::Client& Get() const { return *_Client; }
private:
	std::unique_ptr<Inworld::Client> _Client;
};

#define EMPTY_ARG_RETURN(Arg, Return) INWORLD_WARN_AND_RETURN_EMPTY(LogInworldAIClient, UInworldClient, Arg, Return)
#define NO_CLIENT_RETURN(Return) EMPTY_ARG_RETURN(Client, Return)

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

static FString ToHex(const TArray<uint8>& Data)
{
	std::string result(Data.Num() * 2, '0');
	for (int32 i = 0; i < Data.Num(); i++)
	{
		FCStringAnsi::Sprintf((char*)(result.data()) + (i * 2), "%02x", Data[i]);
	}

	return FString(UTF8_TO_TCHAR(result.c_str()));
}

UInworldClient::UInworldClient()
	: Super()
{
	// Ensure dependencies are loaded
	FInworldAINDKModule::Get();

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

	Client = MakeUnique<NDKClientImpl>();
	Client->Get().InitClientAsync(Sdk,
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

	Client->Get().SetPerceivedLatencyTrackerCallback([this](const std::string& InteractionId, uint32_t LatencyMs)
		{
			OnPerceivedLatencyDelegateNative.Broadcast(UTF8_TO_TCHAR(InteractionId.c_str()), LatencyMs);
			OnPerceivedLatencyDelegate.Broadcast(UTF8_TO_TCHAR(InteractionId.c_str()), LatencyMs);
		}
	);

#if !UE_BUILD_SHIPPING
	auto OnAudioDumperCVarChangedCallback = [this](bool bEnable, FString Path)
		{
			NO_CLIENT_RETURN(void())
			if (Client->Get().GetConnectionState() == Inworld::Client::ConnectionState::Idle)
			{
				return;
			}
		
			const std::string DumpPath = TCHAR_TO_UTF8(*CVarSoundDumpPath.GetValueOnGameThread());
			if (bEnable)
			{
				Client->Get().EnableAudioDump(DumpPath);
			}
			else
			{
				Client->Get().DisableAudioDump();
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
	Client.Reset();
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

	Data = HmacSha256(Data, Data);

	return ToHex(Data);
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

static Inworld::ClientOptions CreateClientOptions(const FInworldPlayerProfile& PlayerProfile, const FInworldCapabilitySet& CapabilitySet, const TMap<FString, FString>& Metadata, const FString& WorkspaceOverride, const FInworldAuth& AuthOverride)
{
	Inworld::ClientOptions Options;

	const UInworldAIClientSettings* InworldAIClientSettings = GetDefault<UInworldAIClientSettings>();
	Options.ServerUrl = TCHAR_TO_UTF8(*InworldAIClientSettings->Environment.TargetUrl);

	if (!WorkspaceOverride.IsEmpty())
	{
		Options.Resource = TCHAR_TO_UTF8(*(FString("workspaces/") + WorkspaceOverride));
	}
	else if (!InworldAIClientSettings->Workspace.IsEmpty())
	{
		Options.Resource = TCHAR_TO_UTF8(*(FString("workspaces/") + InworldAIClientSettings->Workspace));
	}
	
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

void UInworldClient::StartSessionFromScene(const FInworldScene& Scene, const FInworldPlayerProfile& PlayerProfile, const FInworldCapabilitySet& CapabilitySet, const TMap<FString, FString>& Metadata, const FString& WorkspaceOverride, const FInworldAuth& AuthOverride)
{
	Inworld::ClientOptions Options = CreateClientOptions(PlayerProfile, CapabilitySet, Metadata, WorkspaceOverride, AuthOverride);
	
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

	// Failed to parse from settings
	// possible to contain in long scene name
	if (Options.Resource.empty())
	{
		const FString WorkspaceId = [Scene]() -> FString
			{
				// Use first segment of scene for resource
				// 'workspaces/sample-workspace'
				TArray<FString> Split;
				Scene.Name.ParseIntoArray(Split, TEXT("/"));
				if (Split.Num() >= 2)
				{
					return FString(Split[0] + "/" + Split[1]);
				}
				return {};
			}
		();
		Options.Resource = TCHAR_TO_UTF8(*WorkspaceId);
	}

	const std::string SceneId = Options.Resource + "/" + TCHAR_TO_UTF8(*SceneName);

	Client->Get().SetOptions(Options);
	Client->Get().StartClientFromSceneId(SceneId);
}

void UInworldClient::StartSessionFromSave(const FInworldSave& Save, const FInworldPlayerProfile& PlayerProfile, const FInworldCapabilitySet& CapabilitySet, const TMap<FString, FString>& Metadata, const FString& WorkspaceOverride, const FInworldAuth& AuthOverride)
{
	Inworld::SessionSave SessionSave;
	SessionSave.State.resize(Save.State.Num());
	FMemory::Memcpy((uint8*)SessionSave.State.data(), (uint8*)Save.State.GetData(), SessionSave.State.size());

	Client->Get().SetOptions(CreateClientOptions(PlayerProfile, CapabilitySet, Metadata, WorkspaceOverride, AuthOverride));
	Client->Get().StartClientFromSave(SessionSave);
}

void UInworldClient::StartSessionFromToken(const FInworldToken& Token, const FInworldPlayerProfile& PlayerProfile, const FInworldCapabilitySet& CapabilitySet, const TMap<FString, FString>& Metadata, const FString& WorkspaceOverride, const FInworldAuth& AuthOverride)
{
	Inworld::SessionToken SessionToken;
	SessionToken.Token = TCHAR_TO_UTF8(*Token.Token);
	SessionToken.ExpirationTime = Token.ExpirationTime;
	SessionToken.SessionId = TCHAR_TO_UTF8(*Token.SessionId);

	Client->Get().SetOptions(CreateClientOptions(PlayerProfile, CapabilitySet, Metadata, WorkspaceOverride, AuthOverride));
	Client->Get().StartClientFromToken(SessionToken);
}

void UInworldClient::StopSession()
{
	NO_CLIENT_RETURN(void())

	OnPreStopDelegateNative.Broadcast();
	OnPreStopDelegate.Broadcast();

	Client->Get().StopClient();
}

void UInworldClient::PauseSession()
{
	NO_CLIENT_RETURN(void())

	OnPrePauseDelegateNative.Broadcast();
	OnPrePauseDelegate.Broadcast();

	Client->Get().PauseClient();
}

void UInworldClient::ResumeSession()
{
	NO_CLIENT_RETURN(void())

	Client->Get().ResumeClient();
}

void UInworldClient::SaveSession(FOnInworldSessionSavedCallback Callback)
{
	NO_CLIENT_RETURN(void())

	Client->Get().SaveSessionStateAsync([Callback](const std::string& State, bool bSuccess)
		{
			FInworldSave Save;
			if (bSuccess)
			{
				Save.State.SetNumUninitialized(State.size());
				FMemory::Memcpy(Save.State.GetData(), (uint8*)State.data(), Save.State.Num());
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

void UInworldClient::SendInteractionFeedback(const FString& InteractionId, bool bIsLike, const FString& Message)
{
	NO_CLIENT_RETURN(void())
	EMPTY_ARG_RETURN(InteractionId, void())

	Inworld::InteractionFeedback InteractionFeedback;
	InteractionFeedback.bIsLike = true;
	InteractionFeedback.comment = TCHAR_TO_UTF8(*Message);
	std::string interaction = TCHAR_TO_UTF8(*InteractionId);
	Client->Get().SendFeedbackAsync(interaction, InteractionFeedback);
}

void UInworldClient::LoadCharacters(const TArray<FString>& Ids)
{
	NO_CLIENT_RETURN(void())
	EMPTY_ARG_RETURN(Ids, void())

	Client->Get().LoadCharacters(ToStd(Ids));
}

void UInworldClient::UnloadCharacters(const TArray<FString>& Ids)
{
	NO_CLIENT_RETURN(void())
	EMPTY_ARG_RETURN(Ids, void())

	Client->Get().UnloadCharacters(ToStd(Ids));
}

FString UInworldClient::UpdateConversation(const FString& ConversationId, const TArray<FString>& AgentIds, bool bIncludePlayer)
{
	NO_CLIENT_RETURN({})

	if (AgentIds.Num() == 0)
	{
		return {};
	}

	auto Packet = Client->Get().UpdateConversation(ToStd(AgentIds), TCHAR_TO_UTF8(*ConversationId), bIncludePlayer);
	return UTF8_TO_TCHAR(Packet->_Routing._ConversationId.c_str());
}

EInworldConnectionState UInworldClient::GetConnectionState() const
{
	NO_CLIENT_RETURN(EInworldConnectionState::Idle)

	return static_cast<EInworldConnectionState>(Client->Get().GetConnectionState());
}

void UInworldClient::GetConnectionError(FString& OutErrorMessage, int32& OutErrorCode, FInworldConnectionErrorDetails& OutErrorDetails) const
{
	NO_CLIENT_RETURN(void())

	std::string ErrorMessage;
	int32_t ErrorCode;
	Inworld::ErrorDetails ErrorDetails;

	Client->Get().GetConnectionError(ErrorMessage, ErrorCode, ErrorDetails);

	OutErrorMessage = UTF8_TO_TCHAR(ErrorMessage.c_str());
	OutErrorCode = ErrorCode;
	OutErrorDetails.ConnectionErrorType = static_cast<EInworldConnectionErrorType>(ErrorDetails.Error);
	OutErrorDetails.ReconnectionType = static_cast<EInworldReconnectionType>(ErrorDetails.Reconnect);
	OutErrorDetails.ReconnectTime = ErrorDetails.ReconnectTime;
	OutErrorDetails.MaxRetries = ErrorDetails.MaxRetries;
}

FInworldToken UInworldClient::GetSessionToken() const
{
	NO_CLIENT_RETURN({})

	FInworldToken Token;
	Inworld::SessionToken SessionToken = Client->Get().GetSessionToken();
	Token.Token = UTF8_TO_TCHAR(SessionToken.Token.c_str());
	Token.ExpirationTime = SessionToken.ExpirationTime;
	Token.SessionId = UTF8_TO_TCHAR(SessionToken.SessionId.c_str());
	return Token;
}

FInworldCapabilitySet UInworldClient::GetCapabilities() const
{
	NO_CLIENT_RETURN({})

	FInworldCapabilitySet CapabilitySet;
	ConvertCapabilities(Client->Get().GetOptions().Capabilities, CapabilitySet);
	return CapabilitySet;
}

FInworldWrappedPacket UInworldClient::SendTextMessage(const FString& AgentId, const FString& Text)
{
	NO_CLIENT_RETURN({})
	EMPTY_ARG_RETURN(AgentId, {})
	EMPTY_ARG_RETURN(Text, {})

	auto Packet = Client->Get().SendTextMessage(TCHAR_TO_UTF8(*AgentId), TCHAR_TO_UTF8(*Text));
	InworldPacketTranslator PacketTranslator;
	Packet->Accept(PacketTranslator);
	return PacketTranslator.GetPacket();
}

FInworldWrappedPacket UInworldClient::SendTextMessageToConversation(const FString& ConversationId, const FString& Text)
{
	NO_CLIENT_RETURN({})
	EMPTY_ARG_RETURN(ConversationId, {})
	EMPTY_ARG_RETURN(Text, {})

	auto Packet = Client->Get().SendTextMessageToConversation(TCHAR_TO_UTF8(*ConversationId), TCHAR_TO_UTF8(*Text));
	InworldPacketTranslator PacketTranslator;
	Packet->Accept(PacketTranslator);
	return PacketTranslator.GetPacket();
}

void UInworldClient::InitSpeechProcessor(EInworldPlayerSpeechMode Mode, const FInworldPlayerSpeechOptions& SpeechOptions)
{
	NO_CLIENT_RETURN(void())

	const FString Path = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("InworldAI"))->GetBaseDir(), TEXT("Source/ThirdParty/InworldAINDKLibrary/resource/silero_vad_10_27_2022.onnx"));

	Inworld::ClientSpeechOptions_VAD Options_VAD;
	Options_VAD.VADModelPath = TCHAR_TO_UTF8(*Path);
	Options_VAD.VADProbThreshhold = SpeechOptions.VADProbThreshhold;
	Options_VAD.VADBufferChunksNum = SpeechOptions.VADBufferChunksNum;
	Options_VAD.VADSilenceChunksNum = SpeechOptions.VADSilenceChunksNum;

	switch (Mode)
	{
	case EInworldPlayerSpeechMode::DEFAULT:
		Client->Get().InitSpeechProcessor(Inworld::ClientSpeechOptions_Default{});
		break;
	case EInworldPlayerSpeechMode::VAD_DETECT_ONLY:
		Client->Get().InitSpeechProcessor(Inworld::ClientSpeechOptions_VAD_DetectOnly{ Options_VAD });
		break;
	case EInworldPlayerSpeechMode::VAD_DETECT_AND_FILTER:
		Client->Get().InitSpeechProcessor(Inworld::ClientSpeechOptions_VAD_DetectAndFilterAudio{ Options_VAD });
		break;
	}
}

void UInworldClient::DestroySpeechProcessor()
{
	NO_CLIENT_RETURN(void())

	Client->Get().DestroySpeechProcessor();
}

void UInworldClient::SendSoundMessage(const FString& AgentId, const TArray<uint8>& InputData, const TArray<uint8>& OutputData)
{
	NO_CLIENT_RETURN(void())
	EMPTY_ARG_RETURN(AgentId, void())
	EMPTY_ARG_RETURN(InputData, void())

	if (OutputData.Num() > 0)
	{
		std::string inputdata((char*)InputData.GetData(), InputData.Num());
		Client->Get().SendSoundMessage(TCHAR_TO_UTF8(*AgentId), inputdata);
	}
	else
	{
		std::vector<int16> inputdata((int16*)InputData.GetData(), ((int16*)InputData.GetData()) + (InputData.Num() / 2));
		std::vector<int16> outputdata((int16*)OutputData.GetData(), ((int16*)OutputData.GetData()) + (OutputData.Num() / 2));
		Client->Get().SendSoundMessageWithAEC(TCHAR_TO_UTF8(*AgentId), inputdata, outputdata);
	}
}

void UInworldClient::SendSoundMessageToConversation(const FString& ConversationId, const TArray<uint8>& InputData, const TArray<uint8>& OutputData)
{
	NO_CLIENT_RETURN(void())
	EMPTY_ARG_RETURN(ConversationId, void())
	EMPTY_ARG_RETURN(InputData, void())

	if (OutputData.Num() == 0)
	{
		std::string inputdata((char*)InputData.GetData(), InputData.Num());
		Client->Get().SendSoundMessageToConversation(TCHAR_TO_UTF8(*ConversationId), inputdata);
	}
	else
	{
		std::vector<int16> inputdata((int16*)InputData.GetData(), ((int16*)InputData.GetData()) + (InputData.Num() / 2));
		std::vector<int16> outputdata((int16*)OutputData.GetData(), ((int16*)OutputData.GetData()) + (OutputData.Num() / 2));
		Client->Get().SendSoundMessageWithAECToConversation(TCHAR_TO_UTF8(*ConversationId), inputdata, outputdata);
	}
}

void UInworldClient::SendAudioSessionStart(const FString& AgentId, FInworldAudioSessionOptions SessionOptions)
{
	NO_CLIENT_RETURN(void())
	EMPTY_ARG_RETURN(AgentId, void())

	Inworld::AudioSessionStartPayload AudioPayload;
	AudioPayload.MicMode = static_cast<Inworld::AudioSessionStartPayload::MicrophoneMode>(SessionOptions.MicrophoneMode);
	AudioPayload.UndMode = static_cast<Inworld::AudioSessionStartPayload::UnderstandingMode>(SessionOptions.UnderstandingMode);
	Client->Get().StartAudioSession(TCHAR_TO_UTF8(*AgentId), AudioPayload);
}

void UInworldClient::SendAudioSessionStartToConversation(const FString& ConversationId, FInworldAudioSessionOptions SessionOptions)
{
	NO_CLIENT_RETURN(void())
	EMPTY_ARG_RETURN(ConversationId, void())

	Inworld::AudioSessionStartPayload AudioPayload;
	AudioPayload.MicMode = static_cast<Inworld::AudioSessionStartPayload::MicrophoneMode>(SessionOptions.MicrophoneMode);
	AudioPayload.UndMode = static_cast<Inworld::AudioSessionStartPayload::UnderstandingMode>(SessionOptions.UnderstandingMode);
	Client->Get().StartAudioSessionInConversation(TCHAR_TO_UTF8(*ConversationId), AudioPayload);
}

void UInworldClient::SendAudioSessionStop(const FString& AgentId)
{
	NO_CLIENT_RETURN(void())
	EMPTY_ARG_RETURN(AgentId, void())

	Client->Get().StopAudioSession(TCHAR_TO_UTF8(*AgentId));
}

void UInworldClient::SendAudioSessionStopToConversation(const FString& ConversationId)
{
	NO_CLIENT_RETURN(void())
	EMPTY_ARG_RETURN(ConversationId, void())

	Client->Get().StopAudioSessionInConversation(TCHAR_TO_UTF8(*ConversationId));
}

void UInworldClient::SendTrigger(const FString& AgentId, const FString& Name, const TMap<FString, FString>& Params)
{
	NO_CLIENT_RETURN(void())
	EMPTY_ARG_RETURN(AgentId, void())
	EMPTY_ARG_RETURN(Name, void())

	Client->Get().SendCustomEvent(TCHAR_TO_UTF8(*AgentId), TCHAR_TO_UTF8(*Name), ToStd(Params));
}

void UInworldClient::SendTriggerToConversation(const FString& ConversationId, const FString& Name, const TMap<FString, FString>& Params)
{
	NO_CLIENT_RETURN(void())
	EMPTY_ARG_RETURN(ConversationId, void())
	EMPTY_ARG_RETURN(Name, void())

	Client->Get().SendCustomEventToConversation(TCHAR_TO_UTF8(*ConversationId), TCHAR_TO_UTF8(*Name), ToStd(Params));
}

void UInworldClient::SendChangeSceneEvent(const FString& SceneName)
{
	NO_CLIENT_RETURN(void())
	EMPTY_ARG_RETURN(SceneName, void())

	Client->Get().LoadScene(TCHAR_TO_UTF8(*SceneName));
}

void UInworldClient::SendNarrationEvent(const FString& AgentId, const FString& Content)
{
	NO_CLIENT_RETURN(void())
	EMPTY_ARG_RETURN(AgentId, void())
	EMPTY_ARG_RETURN(Content, void())

	Client->Get().SendNarrationEvent(TCHAR_TO_UTF8(*AgentId), TCHAR_TO_UTF8(*Content));
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

	Client->Get().CancelResponse(TCHAR_TO_UTF8(*AgentId), TCHAR_TO_UTF8(*InteractionId), utteranceIds);
}

void UInworldClient::CreateOrUpdateItems(const TArray<FInworldEntityItem>& Items, const TArray<FString>& AddToEntities)
{
	NO_CLIENT_RETURN(void())
	EMPTY_ARG_RETURN(Items, void())
	EMPTY_ARG_RETURN(AddToEntities, void())

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

	Client->Get().CreateOrUpdateItems(items, ToStd(AddToEntities));
}

void UInworldClient::RemoveItems(const TArray<FString>& ItemIds)
{
	NO_CLIENT_RETURN(void())
	EMPTY_ARG_RETURN(ItemIds, void())

	Client->Get().RemoveItems(ToStd(ItemIds));
}

void UInworldClient::AddItemsInEntities(const TArray<FString>& ItemIds, const TArray<FString>& EntityNames)
{
	NO_CLIENT_RETURN(void())
	EMPTY_ARG_RETURN(ItemIds, void())
	EMPTY_ARG_RETURN(EntityNames, void())

	Client->Get().AddItemsInEntities(ToStd(ItemIds), ToStd(EntityNames));
}

void UInworldClient::RemoveItemsInEntities(const TArray<FString>& ItemIds, const TArray<FString>& EntityNames)
{
	NO_CLIENT_RETURN(void())
	EMPTY_ARG_RETURN(ItemIds, void())
	EMPTY_ARG_RETURN(EntityNames, void())

	Client->Get().RemoveItemsInEntities(ToStd(ItemIds), ToStd(EntityNames));
}

void UInworldClient::ReplaceItemsInEntities(const TArray<FString>& ItemIds, const TArray<FString>& EntityNames)
{
	NO_CLIENT_RETURN(void())
	EMPTY_ARG_RETURN(ItemIds, void())
	EMPTY_ARG_RETURN(EntityNames, void())

	Client->Get().ReplaceItemsInEntities(ToStd(ItemIds), ToStd(EntityNames));
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
