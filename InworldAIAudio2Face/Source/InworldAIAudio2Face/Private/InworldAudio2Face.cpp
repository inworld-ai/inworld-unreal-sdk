// Copyright 2023 Theai, Inc. (DBA Inworld) All Rights Reserved.

#include "InworldAudio2Face.h"
#include "InworldAIAudio2FaceModule.h"
#include "CoreMinimal.h"

THIRD_PARTY_INCLUDES_START
#include "ProtoDisableWarning.h"
#include "anim_controller.pb.h"
#include "anim_controller.grpc.pb.h"
#include "anim_controller.pb.cc"
#include "anim_controller.grpc.pb.cc"
/*
#include "anim_data.pb.h"
#include "anim_data.grpc.pb.h"
#include "anim_data.pb.cc"
#include "anim_data.grpc.pb.cc"

#include "audio_data.pb.h"
#include "audio_data.grpc.pb.h"
#include "audio_data.pb.cc"
#include "audio_data.grpc.pb.cc"
*/
#include "grpcpp/grpcpp.h"
#include "grpc/support/log.h"

#include "Utils/SharedQueue.h"
#include "AsyncRoutine.h"
THIRD_PARTY_INCLUDES_END

THIRD_PARTY_INCLUDES_START
#include "Utils/Log.h"
THIRD_PARTY_INCLUDES_END

#include <iostream>
#include <memory>
#include <string>
#include <atomic>
#include <thread>
#include <optional>

namespace Audio2Face
{
	enum PacketType : uint8 {
		BEGIN = 0,
		MID = 1,
		END = 2,
		AVATAR_POSTURE_VAR = 3,
	};

	struct A2XPacket
	{
		A2XPacket() = default;
		virtual ~A2XPacket() = default;

		virtual void ToProto(nvidia::ace::animation::A2XAudioStream& Proto) {}
	};

	struct A2XAudioStream : public A2XPacket
	{
		A2XAudioStream() = default;
		A2XAudioStream(const std::string& in_chunk, PacketType in_type)
			: A2XPacket()
			, chunk(in_chunk)
			, type(in_type)
		{}

		std::string chunk;
		std::unordered_map<std::string, float> emotion_map;
		std::string posture_var;
		PacketType type;

		virtual void ToProto(::nvidia::ace::animation::A2XAudioStream& Proto) override
		{
			Proto.set_audio_chunk(chunk);
			Proto.set_type(static_cast<nvidia::ace::animation::PacketType>(type));
			// TODO: Set emotion map and posture var?
		}
	};

	struct A2XAnimDataStreamHeader
	{
		A2XAnimDataStreamHeader() = default;
		A2XAnimDataStreamHeader(const nvidia::ace::animation::A2XAnimDataStreamHeader& DataStreamHeader)
		{
			success = DataStreamHeader.success();
			message = DataStreamHeader.message();
		}

		bool success;
		std::string message;
	};

	struct A2XAnimDataStreamContent
	{
		A2XAnimDataStreamContent() = default;
		A2XAnimDataStreamContent(const nvidia::ace::animation::A2XAnimDataStreamContent& DataStreamContent)
		{
			usda = DataStreamContent.usda();
			for (const auto& file : DataStreamContent.files())
			{
				files.emplace(file.first, file.second);
			}
		}

		std::string usda;
		std::map<std::string, std::string> files;
	};

	struct A2XAnimDataStream : public A2XPacket
	{
		A2XAnimDataStream() = default;
		A2XAnimDataStream(const nvidia::ace::animation::A2XAnimDataStream& AnimDataStream)
			: A2XPacket()
		{
			if (AnimDataStream.has_header())
			{
				header = A2XAnimDataStreamHeader(AnimDataStream.header());
			}

			if (AnimDataStream.has_anim_data())
			{
				content = A2XAnimDataStreamContent(AnimDataStream.anim_data());
			}
		}

		std::optional<A2XAnimDataStreamHeader> header;
		std::optional<A2XAnimDataStreamContent> content;
	};

	using ReaderWriter = ::grpc::ClientReaderWriter< ::nvidia::ace::animation::A2XAudioStream, ::nvidia::ace::animation::A2XAnimDataStream>;

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

	class IAsyncRoutine
	{
	public:
		virtual ~IAsyncRoutine() = default;

		virtual void Start(std::string ThreadName, std::unique_ptr<Runnable> Runnable) = 0;
		virtual void Stop() = 0;
		virtual bool IsDone() const = 0;
		virtual bool IsValid() const = 0;
		virtual Runnable* GetRunnable() = 0;
	};

	class AsyncRoutine : public IAsyncRoutine
	{
	public:
		virtual ~AsyncRoutine() { Stop(); }

		virtual void Start(std::string ThreadName, std::unique_ptr<Runnable> Runnable) override
		{
			{
				Stop();
				_Runnable = std::move(Runnable);
				_Thread = std::make_unique<std::thread>(&Runnable::Run, _Runnable.get());
			}
		}
		virtual void Stop() override
		{
			if (_Runnable)
			{
				_Runnable->Stop();
				_Runnable.release();
			}

			if (_Thread)
			{
				_Thread.release();
			}
		}

		virtual bool IsDone() const override
		{
			return _Runnable ? _Runnable->IsDone() : false;
		}

		virtual bool IsValid() const override
		{
			return _Runnable && _Thread;
		}

		virtual Runnable* GetRunnable() override
		{
			return _Runnable.get();
		}

	protected:
		std::unique_ptr<Runnable> _Runnable;
		std::unique_ptr<std::thread> _Thread;
	};

	class FAudio2FaceRunnable : public FRunnable
	{
	public:
		FAudio2FaceRunnable(std::unique_ptr<Runnable> InRunnablePtr)
			: RunnablePtr(std::move(InRunnablePtr))
		{}

		bool IsDone() const { return RunnablePtr ? RunnablePtr->IsDone() : false; };
		bool IsValid() const { return GetTask() != nullptr; }

		Runnable* GetTask() const { return RunnablePtr.get(); }

		virtual uint32 Run() override
		{
			if (ensure(RunnablePtr))
			{
				RunnablePtr->Run();
			}

			return 0;
		}

		virtual void Stop() override
		{
			if (RunnablePtr)
			{
				RunnablePtr->Stop();
			}
		}

	protected:
		std::unique_ptr<Runnable> RunnablePtr;
	};


	class FAsyncRoutine
	{
	public:
		FAsyncRoutine(std::string InThreadName, std::unique_ptr<Runnable> InRunnable)
			: RunnablePtr(MakeUnique<FAudio2FaceRunnable>(std::move(InRunnable)))
			, ThreadName(UTF8_TO_TCHAR(InThreadName.c_str()))
		{}

		virtual ~FAsyncRoutine() { Stop(); }

		void Start()
		{
			Thread.Reset(FRunnableThread::Create(RunnablePtr.Get(), *ThreadName));
		}

		void Stop()
		{
			if (Thread.IsValid())
			{
				Thread->Kill(false);
				Thread.Reset();
			}

			if (RunnablePtr)
			{
				RunnablePtr->Stop();
				RunnablePtr.Reset();
			}
		}

		bool IsValid() const { return RunnablePtr && RunnablePtr->IsValid() && Thread; }
		bool IsDone() const { return RunnablePtr && RunnablePtr->IsValid() && RunnablePtr->IsDone(); }

		Runnable* GetRunnable() { return RunnablePtr ? RunnablePtr->GetTask() : nullptr; }

	protected:
		TUniquePtr<FAudio2FaceRunnable> RunnablePtr;
		FString ThreadName = "";

	private:
		TUniquePtr<FRunnableThread> Thread;
	};


	class FAudio2FaceAsyncRoutine : public IAsyncRoutine
	{
	public:
		virtual void Start(std::string ThreadName, std::unique_ptr<Runnable> Runnable) override
		{
			AsyncRoutinePtr = std::make_shared<FAsyncRoutine>(ThreadName, std::move(Runnable));
			AsyncRoutinePtr->Start();
		}
		virtual void Stop() override { if (AsyncRoutinePtr) AsyncRoutinePtr->Stop(); }
		virtual bool IsDone() const { return AsyncRoutinePtr ? AsyncRoutinePtr->IsDone() : true; }
		virtual bool IsValid() const { return AsyncRoutinePtr ? AsyncRoutinePtr->IsValid() : false; }
		virtual Runnable* GetRunnable() { return AsyncRoutinePtr ? AsyncRoutinePtr->GetRunnable() : nullptr; }

		std::shared_ptr<FAsyncRoutine> AsyncRoutinePtr;
	};

	class RunnableMessaging : public Runnable
	{
	public:
		RunnableMessaging(ReaderWriter& ReaderWriter, std::atomic<bool>& bInHasReaderWriterFinished, TQueue<std::shared_ptr<A2XPacket>>& Packets, std::function<void(const std::shared_ptr<A2XPacket>)> ProcessedCallback = nullptr, std::function<void(const grpc::Status&)> InErrorCallback = nullptr)
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

		TQueue<std::shared_ptr<A2XPacket>>& _Packets;
		std::function<void(const std::shared_ptr<A2XPacket>)> _ProcessedCallback;
		std::function<void(const grpc::Status&)> _ErrorCallback;
	};

	class RunnableRead : public RunnableMessaging
	{
	public:
		RunnableRead(ReaderWriter& ReaderWriter, std::atomic<bool>& bHasReaderWriterFinished, TQueue<std::shared_ptr<A2XPacket>>& Packets, std::function<void(const std::shared_ptr<A2XPacket>)> ProcessedCallback = nullptr, std::function<void(const grpc::Status&)> ErrorCallback = nullptr)
			: RunnableMessaging(ReaderWriter, bHasReaderWriterFinished, Packets, ProcessedCallback, ErrorCallback)
		{}
		virtual ~RunnableRead() = default;

		virtual void Run() override;
	};

	class RunnableWrite : public RunnableMessaging
	{
	public:
		RunnableWrite(ReaderWriter& ReaderWriter, std::atomic<bool>& bHasReaderWriterFinished, TQueue<std::shared_ptr<A2XPacket>>& Packets, std::function<void(const std::shared_ptr<A2XPacket>)> ProcessedCallback = nullptr, std::function<void(const grpc::Status&)> ErrorCallback = nullptr)
			: RunnableMessaging(ReaderWriter, bHasReaderWriterFinished, Packets, ProcessedCallback, ErrorCallback)
		{}
		virtual ~RunnableWrite() = default;

		virtual void Run() override;
	};

	void Runnable::Stop()
	{
		_IsDone = true;
		Deinitialize();
	}

	void RunnableRead::Run()
	{
		while (!_HasReaderWriterFinished)
		{
			::nvidia::ace::animation::A2XAnimDataStream IncomingPacket;
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

			std::shared_ptr<A2XAnimDataStream> packet = std::make_shared<A2XAnimDataStream>(IncomingPacket);

			_Packets.Enqueue(packet);

			_ProcessedCallback(packet);
		}

		_IsDone = true;
	}

	void RunnableWrite::Run()
	{
		while (!_HasReaderWriterFinished && !_Packets.IsEmpty())
		{
			auto packet = *_Packets.Peek();
			::nvidia::ace::animation::A2XAudioStream Event;
			packet->ToProto(Event);
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

			_ProcessedCallback(packet);

			_Packets.Pop();
		}

		_IsDone = true;
	}

}

namespace Inworld
{
	class FAudio2Face
	{
	public:
		explicit FAudio2Face(std::shared_ptr<grpc::Channel> channel)
			: stub(nvidia::ace::animation::A2XServiceInterface::NewStub(channel))
		{}

		void Start()
		{
			_AsyncReadTask = std::make_unique<Audio2Face::FAudio2FaceAsyncRoutine>();
			_AsyncWriteTask = std::make_unique<Audio2Face::FAudio2FaceAsyncRoutine>();
			_ReaderWriter = stub->ConvertAudioToAnimData(&context);
			StartReaderWriter();
		}

		void Stop()
		{
			StopReaderWriter();
		}

		void Update()
		{
			ExecutePendingTasks();
		}

		void SendBegin(const std::string& audio)
		{
			auto packet = std::make_shared<Audio2Face::A2XAudioStream>(audio, Audio2Face::PacketType::BEGIN);
			QueueAudioPacket(packet);
		}

		void SendMid(const std::string& audio)
		{
			auto packet = std::make_shared<Audio2Face::A2XAudioStream>(audio, Audio2Face::PacketType::MID);
			QueueAudioPacket(packet);
		}

		void SendEnd(const std::string& audio)
		{
			auto packet = std::make_shared<Audio2Face::A2XAudioStream>(audio, Audio2Face::PacketType::END);
			QueueAudioPacket(packet);
		}

	private:

		void QueueAudioPacket(std::shared_ptr<Audio2Face::A2XAudioStream> Packet)
		{
			_OutgoingPackets.Enqueue(Packet);

			TryToStartWriteTask();
		}

		void StartReaderWriter()
		{
			const bool bHasPendingWriteTask = _AsyncWriteTask->IsValid() && !_AsyncWriteTask->IsDone();
			const bool bHasPendingReadTask = _AsyncReadTask->IsValid() && !_AsyncReadTask->IsDone();
			if (!bHasPendingWriteTask && !bHasPendingReadTask)
			{
				_ErrorMessage = std::string();
				_ErrorCode = grpc::StatusCode::OK;
				_bHasReaderWriterFinished = false;
				TryToStartReadTask();
				TryToStartWriteTask();
			}
		}

		void StopReaderWriter()
		{
			_bHasReaderWriterFinished = true;

			_AsyncReadTask->Stop();
			_AsyncWriteTask->Stop();
			_ReaderWriter.reset();
		}

		void TryToStartReadTask()
		{
			if (!_ReaderWriter)
			{
				return;
			}

			const bool bHasPendingReadTask = _AsyncReadTask->IsValid() && !_AsyncReadTask->IsDone();
			if (!bHasPendingReadTask)
			{
				_AsyncReadTask->Start(
					"Audio2FaceRead",
					std::make_unique<Audio2Face::RunnableRead>(
						*_ReaderWriter.get(),
						_bHasReaderWriterFinished,
						_IncomingPackets,
						[this](const std::shared_ptr<Audio2Face::A2XPacket> InPacket)
						{
							if (!_bPendingIncomingPacketFlush)
							{
								_bPendingIncomingPacketFlush = true;
								AddTaskToMainThread(
									[this]()
									{
										std::shared_ptr<Audio2Face::A2XPacket> Packet;
										while (_IncomingPackets.Dequeue(Packet))
										{
											if (Packet)
											{
												if (_OnPacketCallback)
												{
													_OnPacketCallback(Packet);
												}
											}
										}
										_bPendingIncomingPacketFlush = false;
									});
							}
						},
						[this](const grpc::Status& Status)
						{
							_ErrorMessage = std::string(Status.error_message().c_str());
							_ErrorCode = Status.error_code();
							UE_LOG(LogInworldAIAudio2Face, Log, TEXT("Inworld Audio2Face: Error Code:%d, Msg: %s"), Status.error_code(), UTF8_TO_TCHAR(Status.error_message().c_str()));
						}
					)
				);
			}
		}

		void TryToStartWriteTask()
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
						"Audio2FaceWrite",
						std::make_unique<Audio2Face::RunnableWrite>(
							*_ReaderWriter.get(),
							_bHasReaderWriterFinished,
							_OutgoingPackets,
							[this](const std::shared_ptr<Audio2Face::A2XPacket> InPacket)
							{
								_ErrorMessage = {};
								_ErrorCode = grpc::StatusCode::OK;
							},
							[this](const grpc::Status& Status)
							{
								_ErrorMessage = std::string(Status.error_message().c_str());
								_ErrorCode = Status.error_code();
								UE_LOG(LogInworldAIAudio2Face, Log, TEXT("Inworld Audio2Face: Error Code:%d, Msg: %s"), Status.error_code(), UTF8_TO_TCHAR(Status.error_message().c_str()));
							}
						)
					);
				}
			}
		}

		void AddTaskToMainThread(std::function<void()> Task)
		{
			_MainThreadTasks.Enqueue(Task);
		}

		void ExecutePendingTasks()
		{
			std::function<void()> Task;
			while (_MainThreadTasks.Dequeue(Task))
			{
				Task();
			}
		}

		std::unique_ptr<nvidia::ace::animation::A2XServiceInterface::Stub> stub;

		grpc::ClientContext context;

		std::unique_ptr< grpc::ClientReaderWriter< ::nvidia::ace::animation::A2XAudioStream, ::nvidia::ace::animation::A2XAnimDataStream>> _ReaderWriter;

		std::string _ErrorMessage = std::string();
		int32_t _ErrorCode = grpc::StatusCode::OK;

		std::atomic<bool> _bHasReaderWriterFinished = false;

		std::unique_ptr<Audio2Face::IAsyncRoutine> _AsyncReadTask;
		std::unique_ptr<Audio2Face::IAsyncRoutine> _AsyncWriteTask;

		TQueue<std::shared_ptr<Audio2Face::A2XPacket>> _OutgoingPackets;
		TQueue<std::shared_ptr<Audio2Face::A2XPacket>> _IncomingPackets;

		std::atomic<bool> _bPendingIncomingPacketFlush = false;

		TQueue<std::function<void()>> _MainThreadTasks;

public:
		std::function<void(std::shared_ptr<Audio2Face::A2XPacket>)> _OnPacketCallback;
	};
}

void FInworldAudio2Face::Start(const FString& URL)
{
	InworldAudio2Face = MakeShared<Inworld::FAudio2Face>(grpc::CreateChannel(std::string(TCHAR_TO_UTF8(*URL)), grpc::InsecureChannelCredentials()));
	InworldAudio2Face->Start();
	InworldAudio2Face->_OnPacketCallback = [this](std::shared_ptr<Audio2Face::A2XPacket> packet)
	{
			Audio2Face::A2XAnimDataStream* AnimDataStream = static_cast<Audio2Face::A2XAnimDataStream*>(packet.get());
			
			if (AnimDataStream->header.has_value())
			{
				auto& header = AnimDataStream->header.value();
				FAudio2FaceAnimDataHeader Header;
				Header.Success = header.success;
				Header.Message = UTF8_TO_TCHAR(header.message.c_str());
				OnInworldAudio2FaceAnimDataHeader.ExecuteIfBound(Header);
			}

			if (AnimDataStream->content.has_value())
			{
				auto& content = AnimDataStream->content.value();
				FAudio2FaceAnimDataContent Content;
				Content.USDA = UTF8_TO_TCHAR(content.usda.c_str());
				for (const auto& File : content.files)
				{
					TArray<uint8> Data((uint8*)File.second.data(), File.second.size());
					Content.Files.Add(UTF8_TO_TCHAR(File.first.c_str()), Data);
				}
				OnInworldAudio2FaceAnimDataContent.ExecuteIfBound(Content);
			}
	};
}

void FInworldAudio2Face::Stop()
{
	InworldAudio2Face->Stop();
	InworldAudio2Face.Reset();
}

void FInworldAudio2Face::Update()
{
	if (InworldAudio2Face)
	{
		InworldAudio2Face->Update();
	}
}

void FInworldAudio2Face::SendAudio(const TArray<uint8>& AudioData)
{
	if (!ToSend.IsSet())
	{
		ToSend = AudioData;
		return;
	}
	!bHasSentAudio ? SendToSend(&Inworld::FAudio2Face::SendBegin) : SendToSend(&Inworld::FAudio2Face::SendMid);
	ToSend = AudioData;
}

void FInworldAudio2Face::EndAudio()
{
	SendToSend(&Inworld::FAudio2Face::SendEnd);
	ToSend.Reset();
	bHasSentAudio = false;
}

void FInworldAudio2Face::SendToSend(SendToSendMethod Method)
{
	const TArray<uint8>& ToSendValue = ToSend.GetValue();
	std::string audio = std::string((char*)ToSendValue.GetData(), ToSendValue.Num());
	((*InworldAudio2Face).*Method)(audio);
	bHasSentAudio = true;
}
