/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldSockets.h"

#include "IPAddress.h"
#include "Common/UdpSocketBuilder.h"
#include "Common/UdpSocketReceiver.h"

#include "InworldAIIntegrationModule.h"


Inworld::FSocketBase::~FSocketBase()
{
	if (Socket)
	{
		Socket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(Socket);
		Socket = nullptr;
	}
}

Inworld::FSocketSend::FSocketSend(const FSocketSettings& Settings)
	: FSocketBase(Settings)
{
	Socket = FUdpSocketBuilder(*Settings.Name)
		.AsReusable()
		.WithBroadcast()
		.AsNonBlocking()
		.WithSendBufferSize(Settings.BufferSize)
		.Build();

	if (!Socket)
	{
		UE_LOG(LogInworldAIIntegration, Error, TEXT("FSocketSend::FSocketSend couldn't build a socket"));
		return;
	}

	if (!Socket->Connect(*Settings.RemoteAddr))
	{
		UE_LOG(LogInworldAIIntegration, Error, TEXT("FSocketSend::FSocketSend couldn't connect"));
		return;
	}

	UE_LOG(LogInworldAIIntegration, Log, TEXT("FSocketSend: created local %s, remote %s"),
		*Settings.LocalAddr->ToString(true), *Settings.RemoteAddr->ToString(true));
}

bool Inworld::FSocketSend::ProcessData(TArray<uint8>& Data)
{
	int32 BytesSent;
	if (!Socket->Send(Data.GetData(), Data.Num(), BytesSent))
	{
		UE_LOG(LogInworldAIIntegration, Error, TEXT("FSocketSend::ProcessData couldn't send %d bytes from %s to %s."),
			Data.Num(), *Settings.LocalAddr->ToString(true), *Settings.RemoteAddr->ToString(true));
		return false;
	}

	return Data.Num() == BytesSent;
}

Inworld::FSocketReceive::FSocketReceive(const FSocketSettings& Settings)
	: FSocketBase(Settings)
	, Receiver(nullptr)
{
	const FIPv4Endpoint Endpoint(Settings.LocalAddr);
	Socket = FUdpSocketBuilder(*Settings.Name)
		.AsNonBlocking()
		.AsReusable()
		.BoundToEndpoint(Endpoint)
		.WithReceiveBufferSize(Settings.BufferSize)
		.Build();

	if (!Socket)
	{
		UE_LOG(LogInworldAIIntegration, Error, TEXT("FSocketReceive::Initialize couldn't build a socket"));
		return;
	}

	// potential NAT punchthrough
	/*if (!Socket->Connect(*Settings.RemoteAddr))
	{
		UE_LOG(LogInworldAIIntegration, Error, TEXT("FSocketSend::FSocketReceive couldn't connect"));
		return;
	}
	const char* Data = "hello";
	int32 BytesSent;
	Socket->Send((uint8*)Data, 6, BytesSent);
	if (BytesSent != 6)
	{
		UE_LOG(LogInworldAIIntegration, Error, TEXT("FSocketSend::FSocketReceive couldn't send"));
		return;
	}*/

	Receiver = new FUdpSocketReceiver(Socket, FTimespan::FromMilliseconds(100), *Settings.Name);
	Receiver->OnDataReceived().BindLambda([this](const FArrayReaderPtr& DataPtr, const FIPv4Endpoint& Endpoint)
		{
			TArray<uint8> NewData;
			NewData.AddUninitialized(DataPtr->TotalSize());
			DataPtr->Serialize(NewData.GetData(), DataPtr->TotalSize());
			DataQueue.Enqueue(NewData);
		});

	Receiver->Start();

	UE_LOG(LogInworldAIIntegration, Log, TEXT("FSocketReceive: created local %s, remote %s"),
		*Settings.LocalAddr->ToString(true), *Settings.RemoteAddr->ToString(true));
}

Inworld::FSocketReceive::~FSocketReceive()
{
	if (Receiver)
	{
		Receiver->Stop();
		delete Receiver;
		Receiver = nullptr;
	}
}

bool Inworld::FSocketReceive::ProcessData(TArray<uint8>& Data)
{
	return DataQueue.Dequeue(Data);
}
