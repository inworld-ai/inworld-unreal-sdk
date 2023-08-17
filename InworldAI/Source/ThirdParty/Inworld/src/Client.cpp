/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "Client.h"
#include "RunnableCommand.h"
#include "Utils/Utils.h"
#include "Utils/Log.h"

#include "grpc/impl/codegen/log.h"

#include <vector>
#include <unordered_map>
#include <functional>
#include <cstring>
#include <algorithm>

// prevent std::min/max errors on windows
#undef min
#undef max



constexpr int64_t gMaxTokenLifespan = 60 * 45; // 45 minutes

const std::string DefaultTargetUrl = "api-engine.inworld.ai:443";

static void GrpcLog(gpr_log_func_args* args)
{
	if (args->severity != GPR_LOG_SEVERITY_ERROR)
	{
		return;
	}

	// skip wrong errors(should be severity=info in grpc code)
	std::string Message(args->message);
	uint32_t Idx = Message.find("CompressMessage:");
	if (Idx == 0)
	{
		return;
	}
	Idx = Message.find("DecompressMessage:");
	if (Idx == 0)
	{
		return;
	}
	
	Inworld::LogError("GRPC %s %s::%d",
		ARG_CHAR(args->message),
		ARG_CHAR(args->file),
		args->line);
}

void Inworld::ClientBase::SendPacket(std::shared_ptr<Inworld::Packet> Packet)
{
	_OutgoingPackets.PushBack(Packet);

	TryToStartWriteTask();
}

std::shared_ptr<Inworld::TextEvent> Inworld::ClientBase::SendTextMessage(const std::string& AgentId, const std::string& Text)
{
	auto Packet = std::make_shared<TextEvent>(Text, Routing::Player2Agent(AgentId));
	SendPacket(Packet);
	return Packet;
}

std::shared_ptr<Inworld::DataEvent> Inworld::ClientBase::SendSoundMessage(const std::string& AgentId, const std::string& Data)
{
	auto Packet = std::make_shared<AudioDataEvent>(Data, Routing::Player2Agent(AgentId));
	SendPacket(Packet);
#ifdef INWORLD_AUDIO_DUMP
	if(bDumpAudio)
		_AudioChunksToDump.PushBack(Data);
#endif
	return Packet;
}

std::shared_ptr<Inworld::DataEvent> Inworld::ClientBase::SendSoundMessageWithAEC(const std::string& AgentId, const std::vector<int16_t>& InputData, const std::vector<int16_t>& OutputData)
{
	std::vector<int16_t> FilteredData = _EchoFilter.FilterAudio(InputData, OutputData);

	std::string Data;
	Data.resize(FilteredData.size() * sizeof(int16_t));
	std::memcpy((void*)Data.data(), (void*)FilteredData.data(), Data.size());

	return SendSoundMessage(AgentId, Data);
}

std::shared_ptr<Inworld::CustomEvent> Inworld::ClientBase::SendCustomEvent(std::string AgentId, const std::string& Name, const std::unordered_map<std::string, std::string>& Params)
{
	auto Packet = std::make_shared<CustomEvent>(Name, Params, Routing::Player2Agent(AgentId));
	SendPacket(Packet);
	return Packet;
}

std::shared_ptr<Inworld::ChangeSceneEvent> Inworld::ClientBase::SendChangeSceneEvent(const std::string& Scene)
{
	auto Packet = std::make_shared<ChangeSceneEvent>(Scene, Routing());
	SendPacket(Packet);
	return Packet;
}

void Inworld::ClientBase::CancelResponse(const std::string& AgentId, const std::string& InteractionId, const std::vector<std::string>& UtteranceIds)
{
	auto Packet = std::make_shared<Inworld::CancelResponseEvent>(
		InteractionId, 
		UtteranceIds, 
		Inworld::Routing(Inworld::Actor(), 
		Inworld::Actor(ai::inworld::packets::Actor_Type_AGENT, AgentId)));
	SendPacket(Packet);
}

void Inworld::ClientBase::SetAudioDumpEnabled(bool bEnabled, const std::string& FileName)
{
#ifdef INWORLD_AUDIO_DUMP
	bDumpAudio = bEnabled;
	_AudioDumpFileName = FileName;
	_AsyncAudioDumper->Stop();
	if (bDumpAudio)
	{
		_AsyncAudioDumper->Start("InworldAudioDumper", std::make_unique<RunnableAudioDumper>(_AudioChunksToDump, _AudioDumpFileName));
		Inworld::Log("ASYNC audio dump STARTING");
	}
#endif
}

void Inworld::ClientBase::StartAudioSession(const std::string& AgentId)
{
	auto Packet = std::make_shared<Inworld::ControlEvent>(ai::inworld::packets::ControlEvent_Action_AUDIO_SESSION_START, Inworld::Routing::Player2Agent(AgentId));
	SendPacket(Packet);
}

void Inworld::ClientBase::StopAudioSession(const std::string& AgentId)
{
	auto Packet = std::make_shared<Inworld::ControlEvent>(ai::inworld::packets::ControlEvent_Action_AUDIO_SESSION_END, Inworld::Routing::Player2Agent(AgentId));
	SendPacket(Packet);
}

void Inworld::ClientBase::InitClient(std::string ClientId, std::string ClientVer, std::function<void(ConnectionState)> ConnectionStateCallback, std::function<void(std::shared_ptr<Inworld::Packet>)> PacketCallback)
{
	gpr_set_log_function(GrpcLog);

	_ClientId = ClientId;
	_ClientVer = ClientVer;

	_OnConnectionStateChangedCallback = ConnectionStateCallback;
	_OnPacketCallback = PacketCallback;

	SetConnectionState(ConnectionState::Idle);
}

void Inworld::ClientBase::GenerateToken(std::function<void()> GenerateTokenCallback)
{
	_OnGenerateTokenCallback = GenerateTokenCallback;

	_AsyncGenerateTokenTask->Start(
		"InworldGenerateTokenTask",
		std::make_unique<RunnableGenerateSessionToken>(
			_ClientOptions.ServerUrl,
			_ClientOptions.ApiKey,
			_ClientOptions.ApiSecret,
			[this](const grpc::Status& Status, const InworldEngine::AccessToken& Token) mutable
			{
				if (_SessionInfo.SessionId.empty())
				{
					_SessionInfo.SessionId = Token.session_id();
				}
				_SessionInfo.Token = Token.token();
				_SessionInfo.ExpirationTime = std::time(0) + std::max(std::min(Token.expiration_time().seconds() - std::time(0), gMaxTokenLifespan), int64_t(0));

				AddTaskToMainThread([this, Status]()
				{
					if (!Status.ok())
					{
						_ErrorMessage = std::string(Status.error_message().c_str());
						_ErrorCode = Status.error_code();
						Inworld::LogError("Generate session token FALURE! %s, Code: %d", ARG_STR(_ErrorMessage), _ErrorCode);
						SetConnectionState(ConnectionState::Failed);
					}
					else
					{
						if (_OnGenerateTokenCallback)
						{
							_OnGenerateTokenCallback();
						}
					}
					_OnGenerateTokenCallback = nullptr;
				});
			}
		)
	);
}

void Inworld::ClientBase::StartClient(const ClientOptions& Options, const SessionInfo& Info, std::function<void(const std::vector<AgentInfo>&)> LoadSceneCallback)
{
	if (_ConnectionState != ConnectionState::Idle && _ConnectionState != ConnectionState::Failed)
	{
		return;
	}
	_ClientOptions = Options;
	_SessionInfo = Info;

	_OnLoadSceneCallback = LoadSceneCallback;

	SetConnectionState(ConnectionState::Connecting);

	if (!_SessionInfo.IsValid())
	{
		GenerateToken([this]()
		{
			LoadScene();
		});
	}
	else
	{
		LoadScene();
	}
}

void Inworld::ClientBase::PauseClient()
{
	if (_ConnectionState != ConnectionState::Connected)
	{
		return;
	}

	StopReaderWriter();

	SetConnectionState(ConnectionState::Paused);
}

void Inworld::ClientBase::ResumeClient()
{
	if (_ConnectionState != ConnectionState::Disconnected && _ConnectionState != ConnectionState::Paused)
	{
		return;
	}

	SetConnectionState(ConnectionState::Reconnecting);

	if (!_SessionInfo.IsValid())
	{
		GenerateToken([this]()
		{
			auto Session = static_cast<RunnableLoadScene*>(_AsyncLoadSceneTask->GetRunnable());
			Session->SetToken(_SessionInfo.Token);
			StartReaderWriter();
		});
	}
	else
	{
		StartReaderWriter();
	}
}

void Inworld::ClientBase::StopClient()
{
	if (_ConnectionState == ConnectionState::Idle)
	{
		return;
	}

	StopReaderWriter();
	_AsyncLoadSceneTask->Stop();
	_ClientOptions = ClientOptions();
	_SessionInfo = SessionInfo();
	SetConnectionState(ConnectionState::Idle);
	Inworld::LogClearSessionId();
}

void Inworld::ClientBase::DestroyClient()
{
	StopClient();
	_AsyncLoadSceneTask->Stop();
#ifdef INWORLD_AUDIO_DUMP
	_AsyncAudioDumper->Stop();
#endif
}

bool Inworld::ClientBase::GetConnectionError(std::string& OutErrorMessage, int32_t& OutErrorCode) const
{
	OutErrorMessage = _ErrorMessage;
	OutErrorCode = _ErrorCode;
	return _ErrorCode != grpc::StatusCode::OK;
}

void Inworld::ClientBase::SetConnectionState(ConnectionState State)
{
	if (_ConnectionState == State)
	{
		return;
	}

	_ConnectionState = State;

	if (_ConnectionState == ConnectionState::Connected || _ConnectionState == ConnectionState::Idle)
	{
		_ErrorMessage = std::string();
		_ErrorCode = grpc::StatusCode::OK;
	}

	if (_OnConnectionStateChangedCallback)
	{
		_OnConnectionStateChangedCallback(_ConnectionState);
	}
}

void Inworld::ClientBase::LoadScene()
{
	if (!_SessionInfo.IsValid())
	{
		return;
	}

	Inworld::LogSetSessionId(_SessionInfo.SessionId);

	_AsyncLoadSceneTask->Start(
		"InworldLoadScene",
		std::make_unique<RunnableLoadScene>(
			_SessionInfo.Token,
			_SessionInfo.SessionId,
			_ClientOptions.ServerUrl,
			_ClientOptions.SceneName,
			_ClientOptions.PlayerName,
			_ClientOptions.UserId,
			_ClientOptions.UserSettings,
			_ClientId,
			_ClientVer,
			_ClientOptions.Capabilities,
			[this](const grpc::Status& Status, const InworldEngine::LoadSceneResponse& Response)
			{
				AddTaskToMainThread([this, Status, Response]() {
					OnSceneLoaded(Status, Response);
				});
			}
		)
	);
}

void Inworld::ClientBase::OnSceneLoaded(const grpc::Status& Status, const InworldEngine::LoadSceneResponse& Response)
{
	if (!_OnLoadSceneCallback)
	{
		return;
	}

	if (!Status.ok())
	{
		_ErrorMessage = std::string(Status.error_message().c_str());
		_ErrorCode = Status.error_code();
		Inworld::LogError("Load scene FALURE! %s, Code: %d", ARG_STR(_ErrorMessage), _ErrorCode);
		SetConnectionState(ConnectionState::Failed);
		return;
	}

	std::vector<AgentInfo> AgentInfos;
	AgentInfos.reserve(Response.agents_size());
	for (int32_t i = 0; i < Response.agents_size(); i++)
	{
		AgentInfo Info;
		Info.BrainName = Response.agents(i).brain_name().c_str();
		Info.AgentId = Response.agents(i).agent_id().c_str();
		Info.GivenName = Response.agents(i).given_name().c_str();
		AgentInfos.push_back(Info);

		Inworld::Log("Character registered: %s, Id: %s, GivenName: %s", ARG_STR(Info.BrainName), ARG_STR(Info.AgentId), ARG_STR(Info.GivenName));
	}

	AgentInfo Info;
	Info.BrainName = "__DUMMY__";
	Info.AgentId = "__DUMMY__";
	Info.GivenName = "__DUMMY__";
	AgentInfos.push_back(Info);

	_OnLoadSceneCallback(AgentInfos);
	_OnLoadSceneCallback = nullptr;

	SetConnectionState(ConnectionState::Connected);
	StartReaderWriter();

	Inworld::Log("Create world SUCCESS!");
}

void Inworld::ClientBase::StartReaderWriter()
{
	const bool bHasPendingWriteTask = _AsyncWriteTask->IsValid() && !_AsyncWriteTask->IsDone();
	const bool bHasPendingReadTask = _AsyncReadTask->IsValid() && !_AsyncReadTask->IsDone();
	if (!bHasPendingWriteTask && !bHasPendingReadTask)
	{
		_ErrorMessage = std::string();
		_ErrorCode = grpc::StatusCode::OK;
		_ReaderWriter = static_cast<RunnableLoadScene*>(_AsyncLoadSceneTask->GetRunnable())->Session();
		_bHasReaderWriterFinished = false;
		TryToStartReadTask();
		TryToStartWriteTask();
	}
}

void Inworld::ClientBase::StopReaderWriter()
{
	_bHasReaderWriterFinished = true;
	auto* Task = static_cast<RunnableLoadScene*>(_AsyncLoadSceneTask->GetRunnable());
	if (Task)
	{
		auto& Context = Task->GetContext();
		if (Context)
		{
			Context->TryCancel();
		}
	}
	_AsyncReadTask->Stop();
	_AsyncWriteTask->Stop();
	_ReaderWriter.reset();
}

void Inworld::ClientBase::TryToStartReadTask()
{
	if (!_ReaderWriter)
	{
		return;
	}

	const bool bHasPendingReadTask = _AsyncReadTask->IsValid() && !_AsyncReadTask->IsDone();
	if (!bHasPendingReadTask)
	{
		_AsyncReadTask->Start(
			"InworldRead",
			std::make_unique<RunnableRead>(
				*_ReaderWriter.get(), 
				_bHasReaderWriterFinished, 
				_IncomingPackets,
				[this](const std::shared_ptr<Inworld::Packet> InPacket)
				{
					if (!_bPendingIncomingPacketFlush)
					{
						_bPendingIncomingPacketFlush = true;
						AddTaskToMainThread(
							[this]()
							{
								std::shared_ptr<Inworld::Packet> Packet;
								while (_IncomingPackets.PopFront(Packet))
								{
									if (Packet)
									{
										_OnPacketCallback(Packet);
									}
								}
								_bPendingIncomingPacketFlush = false;
							});
					}
					if (_ConnectionState != ConnectionState::Connected)
					{
						AddTaskToMainThread(
							[this]()
							{
								SetConnectionState(ConnectionState::Connected);
							});
					}
				},
				[this](const grpc::Status& Status)
				{
					_ErrorMessage = std::string(Status.error_message().c_str());
					_ErrorCode = Status.error_code();
					Inworld::LogError("Message READ failed: %s. Code: %d", ARG_STR(_ErrorMessage), _ErrorCode);
					AddTaskToMainThread(
						[this]()
						{
							SetConnectionState(ConnectionState::Disconnected);
						});
				}
			)
		);
	}
}

void Inworld::ClientBase::TryToStartWriteTask()
{
	if (!_ReaderWriter)
	{
		return;
	}

	const bool bHasPendingWriteTask = _AsyncWriteTask->IsValid() && !_AsyncWriteTask->IsDone();
	if (!bHasPendingWriteTask)
	{
		const bool bHasOutgoingPackets = !_OutgoingPackets.IsEmpty();
		if (bHasOutgoingPackets)
		{
			_AsyncWriteTask->Start(
				"InworldWrite",
				std::make_unique<RunnableWrite>(
					*_ReaderWriter.get(),
					_bHasReaderWriterFinished,
					_OutgoingPackets,
					[this](const std::shared_ptr<Inworld::Packet> InPacket)
					{
						if (_ConnectionState != ConnectionState::Connected)
						{
							AddTaskToMainThread([this]() {
								SetConnectionState(ConnectionState::Connected);
							});
						}
					},
					[this](const grpc::Status& Status)
					{
						_ErrorMessage = std::string(Status.error_message().c_str());
						_ErrorCode = Status.error_code();
						Inworld::LogError("Message WRITE failed: %s. Code: %d", ARG_STR(_ErrorMessage), _ErrorCode);
						AddTaskToMainThread([this]() {
							SetConnectionState(ConnectionState::Disconnected);
						});
					}
				)
			);
		}
	}
}

void Inworld::Client::Update()
{
	ExecutePendingTasks();
}

void Inworld::Client::AddTaskToMainThread(std::function<void()> Task)
{
	_MainThreadTasks.PushBack(Task);
}

void Inworld::Client::ExecutePendingTasks()
{
	std::function<void()> Task;
	while (_MainThreadTasks.PopFront(Task))
	{
		Task();
	}
}

