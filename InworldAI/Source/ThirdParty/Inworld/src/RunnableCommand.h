/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "proto/ProtoDisableWarning.h"

#include <thread>
#include <atomic>
#include <memory>
#include <functional>

#include "grpcpp/impl/codegen/status.h"
#include "grpcpp/impl/codegen/sync_stream.h"
#include "grpcpp/create_channel.h"
#include "world-engine.grpc.pb.h"
#include "ai/inworld/studio/v1alpha/tokens.grpc.pb.h"
#include "ai/inworld/studio/v1alpha/users.grpc.pb.h"
#include "ai/inworld/studio/v1alpha/workspaces.grpc.pb.h"
#include "ai/inworld/studio/v1alpha/scenes.grpc.pb.h"
#include "ai/inworld/studio/v1alpha/characters.grpc.pb.h"
#include "ai/inworld/studio/v1alpha/apikeys.grpc.pb.h"

#include "Utils/Utils.h"
#include "Utils/SharedQueue.h"
#include "Packets.h"

namespace InworldEngine = ai::inworld::engine;
namespace InworldPackets = ai::inworld::packets;
namespace InworldV1alpha = ai::inworld::studio::v1alpha;

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

namespace Inworld
{
	using ReaderWriter = ::grpc::ClientReaderWriter< InworldPackets::InworldPacket, InworldPackets::InworldPacket>;

	class Runnable
	{
	public:
		virtual ~Runnable() = default;

		bool IsDone() const { return _IsDone; };
		void Stop();

		virtual void Run() = 0;
		virtual void Deinitialize() {}

	protected:
		std::atomic<bool> _IsDone = false;
	};

	class RunnableMessaging : public Runnable
	{
	public:
		RunnableMessaging(ReaderWriter& ReaderWriter, std::atomic<bool>& bInHasReaderWriterFinished, SharedQueue<std::shared_ptr<Inworld::Packet>>& Packets, std::function<void(const std::shared_ptr<Inworld::Packet>)> ProcessedCallback = nullptr, std::function<void(const grpc::Status&)> InErrorCallback = nullptr)
			: _ReaderWriter(ReaderWriter)
			, _HasReaderWriterFinished(bInHasReaderWriterFinished)
			, _Packets(Packets)
			, _ProcessedCallback(ProcessedCallback)
			, _ErrorCallback(InErrorCallback)
		{}
		virtual ~RunnableMessaging() = default;

	protected:
		ReaderWriter& _ReaderWriter;
		std::atomic<bool>& _HasReaderWriterFinished;

		SharedQueue<std::shared_ptr<Inworld::Packet>>& _Packets;
		std::function<void(const std::shared_ptr<Inworld::Packet>)> _ProcessedCallback;
		std::function<void(const grpc::Status&)> _ErrorCallback;
	};

	class RunnableRead : public RunnableMessaging
	{
	public:
		RunnableRead(ReaderWriter& ReaderWriter, std::atomic<bool>& bHasReaderWriterFinished, SharedQueue<std::shared_ptr<Inworld::Packet>>& Packets, std::function<void(const std::shared_ptr<Inworld::Packet>)> ProcessedCallback = nullptr, std::function<void(const grpc::Status&)> ErrorCallback = nullptr)
			: RunnableMessaging(ReaderWriter, bHasReaderWriterFinished, Packets, ProcessedCallback, ErrorCallback)
		{}
		virtual ~RunnableRead() = default;

		virtual void Run() override;
	};

	class RunnableWrite : public RunnableMessaging
	{
	public:
		RunnableWrite(ReaderWriter& ReaderWriter, std::atomic<bool>& bHasReaderWriterFinished, SharedQueue<std::shared_ptr<Inworld::Packet>>& Packets, std::function<void(const std::shared_ptr<Inworld::Packet>)> ProcessedCallback = nullptr, std::function<void(const grpc::Status&)> ErrorCallback = nullptr)
			: RunnableMessaging(ReaderWriter, bHasReaderWriterFinished, Packets, ProcessedCallback, ErrorCallback)
		{}
		virtual ~RunnableWrite() = default;

		virtual void Run() override;
	};

	template<typename TService, class TResponse>
	class RunnableRequest : public Runnable
	{
	public:
		RunnableRequest(const std::string& ServerUrl, std::function<void(const grpc::Status& Status, const TResponse& Response)> Callback = nullptr)
			: _ServerUrl(ServerUrl)
			, _Callback(Callback)
		{}
		virtual ~RunnableRequest() = default;

		virtual grpc::Status RunProcess() = 0;

		virtual void Run() override
		{
			_Status = RunProcess();

			if (_Callback)
			{
				_Callback(_Status, _Response);
			}

			_IsDone = true;
		}

		virtual void Deinitialize() override
		{
			if (_Context)
			{
				_Context->TryCancel();
			}
		}

		grpc::Status& GetStatus() { return _Status; }
		TResponse& GetResponse() { return _Response; }
		std::unique_ptr<ClientContext>& GetContext() { return _Context; }

	protected:
		struct FHeader
		{
			std::string Key;
			std::string Value;
		};

		std::unique_ptr<typename TService::Stub>& CreateStub()
		{
            grpc::SslCredentialsOptions SslCredentialsOptions;
            SslCredentialsOptions.pem_root_certs = Utils::GetSslRootCerts();
			_Stub = TService::NewStub(grpc::CreateChannel(_ServerUrl, grpc::SslCredentials(SslCredentialsOptions)));
			return _Stub;
		}

		std::unique_ptr<ClientContext>& UpdateContext(const std::vector<FHeader>& Headers)
		{
			_Context = std::make_unique<ClientContext>();
			for (auto& Header : Headers)
			{
				_Context->AddMetadata(Header.Key, Header.Value);
			}
			return _Context;
		}

		std::unique_ptr<typename TService::Stub> _Stub;
		grpc::Status _Status;
		TResponse _Response;

		std::string _ServerUrl;
		std::unique_ptr<ClientContext> _Context;

		std::function<void(const grpc::Status& Status, const TResponse& Response)> _Callback;
	};

	class RunnableGenerateSessionToken : public RunnableRequest<InworldEngine::WorldEngine, InworldEngine::AccessToken>
	{
	public:
		RunnableGenerateSessionToken(const std::string& ServerUrl, const std::string& ApiKey, const std::string& ApiSecret, std::function<void(const grpc::Status&, const InworldEngine::AccessToken&)> Callback = nullptr)
			: RunnableRequest(ServerUrl, Callback)
			, _ApiKey(ApiKey)
			, _ApiSecret(ApiSecret)
		{}

		virtual ~RunnableGenerateSessionToken() = default;

		virtual grpc::Status RunProcess() override;

	private:
		std::string GenerateHeader() const;

		std::string _ApiKey;
		std::string _ApiSecret;
	};

	struct CapabilitySet
	{
		bool Animations = false;
		bool Text = false;
		bool Audio = false;
		bool Emotions = false;
		bool Gestures = false;
		bool Interruptions = false;
		bool Triggers = false;
		bool EmotionStreaming = false;
		bool SilenceEvents = false;
		bool PhonemeInfo = false;
		bool LoadSceneInSession = false;
		bool Continuation = true;
		bool TurnBasedSTT = true;
		bool NarratedActions = true;
	};

	struct UserSettings
	{
		struct PlayerProfile
		{
			struct PlayerField
			{
				std::string Id;
				std::string Value;
			};

			std::vector<PlayerField> Fields;
		};

		PlayerProfile Profile;
	};

	class RunnableLoadScene : public RunnableRequest<InworldEngine::WorldEngine, InworldEngine::LoadSceneResponse>
	{
	public:
		RunnableLoadScene(const std::string& Token, const std::string& SessionId, const std::string& ServerUrl, const std::string& SceneName, const std::string& PlayerName, const std::string& UserId, const UserSettings& UserSettings, const std::string& ClientId, const std::string& ClientVersion, const CapabilitySet& Capabilities, std::function<void(const grpc::Status&, const InworldEngine::LoadSceneResponse&)> Callback = nullptr)
			: RunnableRequest(ServerUrl, Callback)
			, _Token(Token)
			, _SessionId(SessionId)
			, _SceneName(SceneName)
			, _PlayerName(PlayerName)
			, _UserId(UserId)
			, _UserSettings(UserSettings)
			, _ClientId(ClientId)
			, _ClientVersion(ClientVersion)
			, _Capabilities(Capabilities)
		{}

		virtual ~RunnableLoadScene() = default;

		virtual grpc::Status RunProcess() override;

		std::unique_ptr<ReaderWriter> Session();

		void SetToken(const std::string& InToken)
		{
			_Token = InToken;
		}

	private:
		std::string _Token;
		std::string _SessionId;
		std::string _LoadSceneUrl;
		std::string _SceneName;
		std::string _PlayerName;
		std::string _UserId;
		UserSettings _UserSettings;
		std::string _ClientId;
		std::string _ClientVersion;
		CapabilitySet _Capabilities;
	};

	class RunnableGenerateUserTokenRequest : public RunnableRequest<InworldV1alpha::Users, InworldV1alpha::GenerateTokenUserResponse>
	{
	public:
		RunnableGenerateUserTokenRequest(const std::string& InFirebaseToken, const std::string& InServerUrl, std::function<void(const grpc::Status& Status, const InworldV1alpha::GenerateTokenUserResponse& Response)> InCallback)
			: RunnableRequest(InServerUrl, InCallback)
			, _FirebaseToken(InFirebaseToken)
		{}
		virtual ~RunnableGenerateUserTokenRequest() = default;

		virtual grpc::Status RunProcess() override;

	private:
		std::string _FirebaseToken;
	};

	std::unique_ptr<Runnable> MakeRunnableGenerateUserTokenRequest(const std::string& InFirebaseToken, const std::string& InServerUrl, std::function<void(const grpc::Status& Status, const InworldV1alpha::GenerateTokenUserResponse& Response)> InCallback);

	class RunnableListWorkspacesRequest : public RunnableRequest<InworldV1alpha::Workspaces, InworldV1alpha::ListWorkspacesResponse>
	{
	public:
		RunnableListWorkspacesRequest(const std::string& InInworldToken, const std::string& InServerUrl, std::function<void(const grpc::Status& Status, const InworldV1alpha::ListWorkspacesResponse& Response)> InCallback)
			: RunnableRequest(InServerUrl, InCallback)
			, _InworldToken(InInworldToken)
		{}
		virtual ~RunnableListWorkspacesRequest() = default;

		virtual grpc::Status RunProcess() override;

	private:
		std::string _InworldToken;
	};

	std::unique_ptr<Runnable> MakeRunnableListWorkspacesRequest(const std::string& InInworldToken, const std::string& InServerUrl, std::function<void(const grpc::Status& Status, const InworldV1alpha::ListWorkspacesResponse& Response)> InCallback);

	class RunnableListScenesRequest : public RunnableRequest<InworldV1alpha::Scenes, InworldV1alpha::ListScenesResponse>
	{
	public:
		RunnableListScenesRequest(const std::string& InInworldToken, const std::string& InServerUrl, const std::string& InWorkspace, std::function<void(const grpc::Status& Status, const InworldV1alpha::ListScenesResponse& Response)> InCallback)
			: RunnableRequest(InServerUrl, InCallback)
			, _InworldToken(InInworldToken)
			, _Workspace(InWorkspace)
		{}
		virtual ~RunnableListScenesRequest() = default;

		virtual grpc::Status RunProcess() override;

	private:
		std::string _InworldToken;
		std::string _Workspace;
	};

	std::unique_ptr<Runnable> MakeRunnableListScenesRequest(const std::string& InInworldToken, const std::string& InServerUrl, const std::string& InWorkspace, std::function<void(const grpc::Status& Status, const InworldV1alpha::ListScenesResponse& Response)> InCallback);

	class RunnableListCharactersRequest : public RunnableRequest<InworldV1alpha::Characters, InworldV1alpha::ListCharactersResponse>
	{
	public:
		RunnableListCharactersRequest(const std::string& InInworldToken, const std::string& InServerUrl, const std::string& InWorkspace, std::function<void(const grpc::Status& Status, const InworldV1alpha::ListCharactersResponse& Response)> InCallback)
			: RunnableRequest(InServerUrl, InCallback)
			, _InworldToken(InInworldToken)
			, _Workspace(InWorkspace)
		{}
		virtual ~RunnableListCharactersRequest() = default;

		virtual grpc::Status RunProcess() override;

	private:
		std::string _InworldToken;
		std::string _Workspace;
	};

	std::unique_ptr<Runnable> MakeRunnableListCharactersRequest(const std::string& InInworldToken, const std::string& InServerUrl, const std::string& InWorkspace, std::function<void(const grpc::Status& Status, const InworldV1alpha::ListCharactersResponse& Response)> InCallback);

	class RunnableListApiKeysRequest : public RunnableRequest<InworldV1alpha::ApiKeys, InworldV1alpha::ListApiKeysResponse>
	{
	public:
		RunnableListApiKeysRequest(const std::string& InInworldToken, const std::string& InServerUrl, const std::string& InWorkspace, std::function<void(const grpc::Status& Status, const InworldV1alpha::ListApiKeysResponse& Response)> InCallback)
			: RunnableRequest(InServerUrl, InCallback)
			, _InworldToken(InInworldToken)
			, _Workspace(InWorkspace)
		{}
		virtual ~RunnableListApiKeysRequest() = default;

		virtual grpc::Status RunProcess() override;

	private:
		std::string _InworldToken;
		std::string _Workspace;
	};

	std::unique_ptr<Runnable> MakeRunnableListApiKeysRequest(const std::string& InInworldToken, const std::string& InServerUrl, const std::string& InWorkspace, std::function<void(const grpc::Status& Status, const InworldV1alpha::ListApiKeysResponse& Response)> InCallback);

}
