/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "RunnableCommand.h"
#include "Packets.h"
#include "Utils/Utils.h"
#include "Utils/Log.h"

#include <iomanip>
#include <random>
#include <sstream>

void Inworld::Runnable::Stop()
{
	_IsDone = true;
	Deinitialize();
}

void Inworld::RunnableRead::Run()
{
	while (!_HasReaderWriterFinished)
	{
		InworldPackets::InworldPacket IncomingPacket;
		if (!_ReaderWriter.Read(&IncomingPacket))
		{
			if (!_HasReaderWriterFinished)
			{
				_HasReaderWriterFinished = true;
				_ErrorCallback(_ReaderWriter.Finish());
			}

			_IsDone = true;

			return;
		}

		std::shared_ptr<Inworld::Packet> Packet;
		// Text event
		if (IncomingPacket.has_text())
		{
			Packet = std::make_shared<Inworld::TextEvent>(IncomingPacket);
		}
		else if (IncomingPacket.has_data_chunk())
		{
			// Audio response with Uncompressed 16-bit signed little-endian samples (Linear PCM) data.
			if (IncomingPacket.data_chunk().type() == ai::inworld::packets::DataChunk_DataType_AUDIO)
			{
				Packet = std::make_shared<Inworld::AudioDataEvent>(IncomingPacket);
			}
		}
		else if (IncomingPacket.has_control())
		{
			Packet = std::make_shared<Inworld::ControlEvent>(IncomingPacket);
		}
		else if (IncomingPacket.has_emotion())
		{
			Packet = std::make_shared<Inworld::EmotionEvent>(IncomingPacket);
		}
		else if (IncomingPacket.has_custom())
		{
			Packet = std::make_shared<Inworld::CustomEvent>(IncomingPacket);
		}
		else if (IncomingPacket.has_load_scene_output())
		{
			Packet = std::make_shared<Inworld::ChangeSceneEvent>(IncomingPacket);
		}
		else
		{
			// Unknown packet type
			continue;
		}

		_Packets.PushBack(Packet);

		_ProcessedCallback(Packet);
	}

	_IsDone = true;
}

void Inworld::RunnableWrite::Run()
{
	while (!_HasReaderWriterFinished && !_Packets.IsEmpty())
	{
		auto Packet = _Packets.Front();
		InworldPackets::InworldPacket Event = Packet->ToProto();
		if (!_ReaderWriter.Write(Event))
		{
			if (!_HasReaderWriterFinished)
			{
				_HasReaderWriterFinished = true;
				_ErrorCallback(_ReaderWriter.Finish());
			}

			_IsDone = true;

			return;
		}

		_Packets.PopFront();

		_ProcessedCallback(Packet);
	}

	_IsDone = true;
}

#ifdef INWORLD_AUDIO_DUMP
void Inworld::RunnableAudioDumper::Run()
{
	AudioDumper.OnSessionStart(FileName);

	while (!_IsDone)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		std::string Chunk;
		while (AudioChuncks.PopFront(Chunk))
		{
			AudioDumper.OnMessage(Chunk);
		}
	}

	AudioDumper.OnSessionStop();
}
#endif

grpc::Status Inworld::RunnableGenerateSessionToken::RunProcess()
{
	InworldEngine::GenerateTokenRequest AuthRequest;
	AuthRequest.set_key(_ApiKey);

	auto& AuthCtx = UpdateContext({ {"authorization", GenerateHeader() } });

	return CreateStub()->GenerateToken(AuthCtx.get(), AuthRequest, &_Response);
}

grpc::Status Inworld::RunnableLoadScene::RunProcess()
{
	InworldEngine::LoadSceneRequest LoadSceneRequest;
	LoadSceneRequest.set_name(_SceneName);

	auto* Capabilities = LoadSceneRequest.mutable_capabilities();
	Capabilities->set_text(_Capabilities.Text);
	Capabilities->set_audio(_Capabilities.Audio);
	Capabilities->set_emotions(_Capabilities.Emotions);
	Capabilities->set_gestures(_Capabilities.Gestures);
	Capabilities->set_interruptions(_Capabilities.Interruptions);
	Capabilities->set_triggers(_Capabilities.Triggers);
	Capabilities->set_emotion_streaming(_Capabilities.EmotionStreaming);
	Capabilities->set_silence_events(_Capabilities.SilenceEvents);
	Capabilities->set_phoneme_info(_Capabilities.PhonemeInfo);
	Capabilities->set_load_scene_in_session(_Capabilities.LoadSceneInSession);
	Capabilities->set_narrated_actions(_Capabilities.NarratedActions);
	Capabilities->set_continuation(_Capabilities.Continuation);
	Capabilities->set_turn_based_stt(_Capabilities.TurnBasedSTT);
	

	auto* User = LoadSceneRequest.mutable_user();
	User->set_id(_UserId);
	User->set_name(_PlayerName);

	Inworld::Log("RunnableLoadScene User id: %s", ARG_STR(_UserId));

	auto* Client = LoadSceneRequest.mutable_client();
	Client->set_id(_ClientId);
	Client->set_version(_ClientVersion);

	auto* UserSettings = LoadSceneRequest.mutable_user_settings();
	auto* PlayerProfile = UserSettings->mutable_player_profile();
	for (const auto& Field : _UserSettings.Profile.Fields)
	{
		PlayerProfile->add_fields();
		auto* PlayerField = PlayerProfile->mutable_fields(PlayerProfile->fields_size() - 1);
		PlayerField->set_field_id(Field.Id);
		PlayerField->set_field_value(Field.Value);
	}

	auto& Ctx = UpdateContext({
		{ "authorization", std::string("Bearer ") + _Token },
		{ "session-id", _SessionId }
		});

	return CreateStub()->LoadScene(Ctx.get(), LoadSceneRequest, &_Response);
}

std::unique_ptr<Inworld::ReaderWriter> Inworld::RunnableLoadScene::Session()
{
	auto& Ctx = UpdateContext({
			{ "authorization", std::string("Bearer ") + _Token },
			{ "session-id", _SessionId }
		});
	return _Stub->Session(Ctx.get());
}

std::string Inworld::RunnableGenerateSessionToken::GenerateHeader() const
{
	std::time_t Time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	std::tm Tm = *std::gmtime(&Time);
	std::stringstream Stream;
	Stream << std::put_time(&Tm, "%Y%m%d%H%M%S");
	const std::string CurrentTime = Stream.str();

	const std::string Chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890";
	std::string Nonce = "00000000000";

	std::random_device Rd;
	std::mt19937 Gen(Rd());
	std::uniform_int_distribution<> Distr(0, Chars.size() - 1);

	for (int i = 0; i < Nonce.size(); i++)
	{
		Nonce[i] = Chars[Distr(Gen)];
	}

	std::vector<std::string> CryptoArgs = {
		CurrentTime,
		"api-engine.inworld.ai",
		"ai.inworld.engine.WorldEngine/GenerateToken",
		Nonce,
		"iw1_request"
	};

	const std::string IniKey = std::string("IW1") + _ApiSecret;
	std::vector<uint8_t> Key;
	Key.resize(IniKey.size());
	memcpy(Key.data(), IniKey.data(), IniKey.size());

	for (const auto& Arg : CryptoArgs)
	{
		std::vector<uint8_t> Data;
		Data.resize(Arg.size());
		memcpy(Data.data(), Arg.data(), Arg.size());
		Key = Inworld::Utils::HmacSha256(Data, Key);
	}

	const std::string Signature = Inworld::Utils::ToHex(Key);

	return 
		std::string("IW1-HMAC-SHA256 ApiKey=") + _ApiKey + 
		",DateTime=" + CurrentTime +
		",Nonce=" + Nonce + 
		",Signature=" + Signature;
}

grpc::Status Inworld::RunnableGenerateUserTokenRequest::RunProcess()
{
	InworldV1alpha::GenerateTokenUserRequest Request;
	Request.set_type(InworldV1alpha::AuthType::AUTH_TYPE_FIREBASE);
	Request.set_token(_FirebaseToken);

	auto& Ctx = UpdateContext({
		{"x-authorization-bearer-type", "firebase"},
		{"authorization", std::string("Bearer ") + _FirebaseToken }
		});

	return CreateStub()->GenerateTokenUser(Ctx.get(), Request, &_Response);
}

std::unique_ptr<Inworld::Runnable> Inworld::MakeRunnableGenerateUserTokenRequest(const std::string& InFirebaseToken, const std::string& InServerUrl, std::function<void(const grpc::Status& Status, const InworldV1alpha::GenerateTokenUserResponse& Response)> InCallback)
{
	return std::make_unique<Inworld::RunnableGenerateUserTokenRequest>(InFirebaseToken, InServerUrl, InCallback);
}

grpc::Status Inworld::RunnableListWorkspacesRequest::RunProcess()
{
	InworldV1alpha::ListWorkspacesRequest Request;

	auto& Ctx = UpdateContext({
		{"x-authorization-bearer-type", "inworld"},
		{"authorization", std::string("Bearer ") + _InworldToken }
		});

	return CreateStub()->ListWorkspaces(Ctx.get(), Request, &_Response);
}

std::unique_ptr<Inworld::Runnable> Inworld::MakeRunnableListWorkspacesRequest(const std::string& InInworldToken, const std::string& InServerUrl, std::function<void(const grpc::Status& Status, const InworldV1alpha::ListWorkspacesResponse& Response)> InCallback)
{
	return std::make_unique<Inworld::RunnableListWorkspacesRequest>(InInworldToken, InServerUrl, InCallback);
}

grpc::Status Inworld::RunnableListScenesRequest::RunProcess()
{
	InworldV1alpha::ListScenesRequest Request;
	Request.set_parent(_Workspace);

	auto& Ctx = UpdateContext({
		{"x-authorization-bearer-type", "inworld"},
		{"authorization", std::string("Bearer ") + _InworldToken }
		});

	return CreateStub()->ListScenes(Ctx.get(), Request, &_Response);
}

std::unique_ptr<Inworld::Runnable> Inworld::MakeRunnableListScenesRequest(const std::string& InInworldToken, const std::string& InServerUrl, const std::string& InWorkspace, std::function<void(const grpc::Status& Status, const InworldV1alpha::ListScenesResponse& Response)> InCallback)
{
	return std::make_unique<Inworld::RunnableListScenesRequest>(InInworldToken, InServerUrl, InWorkspace, InCallback);
}

grpc::Status Inworld::RunnableListCharactersRequest::RunProcess()
{
	InworldV1alpha::ListCharactersRequest Request;
	Request.set_parent(_Workspace);
	Request.set_view(InworldV1alpha::CharacterView::CHARACTER_VIEW_DEFAULT);

	auto& Ctx = UpdateContext({
		{"x-authorization-bearer-type", "inworld"},
		{"authorization", std::string("Bearer ") + _InworldToken }
		});

	return CreateStub()->ListCharacters(Ctx.get(), Request, &_Response);
}

std::unique_ptr<Inworld::Runnable> Inworld::MakeRunnableListCharactersRequest(const std::string& InInworldToken, const std::string& InServerUrl, const std::string& InWorkspace, std::function<void(const grpc::Status& Status, const InworldV1alpha::ListCharactersResponse& Response)> InCallback)
{
	return std::make_unique<Inworld::RunnableListCharactersRequest>(InInworldToken, InServerUrl, InWorkspace, InCallback);
}

grpc::Status Inworld::RunnableListApiKeysRequest::RunProcess()
{
	InworldV1alpha::ListApiKeysRequest Request;
	Request.set_parent(_Workspace);

	auto& Ctx = UpdateContext({
		{"x-authorization-bearer-type", "inworld"},
		{"authorization", std::string("Bearer ") + _InworldToken }
		});

	return CreateStub()->ListApiKeys(Ctx.get(), Request, &_Response);
}

std::unique_ptr<Inworld::Runnable> Inworld::MakeRunnableListApiKeysRequest(const std::string& InInworldToken, const std::string& InServerUrl, const std::string& InWorkspace, std::function<void(const grpc::Status& Status, const InworldV1alpha::ListApiKeysResponse& Response)> InCallback)
{
	return std::make_unique<Inworld::RunnableListApiKeysRequest>(InInworldToken, InServerUrl, InWorkspace, InCallback);
}
