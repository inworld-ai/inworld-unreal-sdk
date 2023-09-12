/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldSockets.h"

#include "IPAddress.h"
#include "Common/UdpSocketBuilder.h"
#include "Common/UdpSocketReceiver.h"
#include "Common/UdpSocketSender.h"

#include "InworldAIIntegrationModule.h"


bool Inworld::FSocketSend::Initialize(const FSocketSettings& Settings)
{
	TSharedRef<FInternetAddr> Addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();

	bool bIsValid;
	Addr->SetIp(*Settings.IpAddr, bIsValid);
	Addr->SetPort(Settings.Port);

	if (!bIsValid)
	{
		UE_LOG(LogInworldAIIntegration, Error, TEXT("FSocketSend::Initialize Address is invalid '%s:%d'"), *Settings.IpAddr, Settings.Port);
		return false;
	}

	Socket = FUdpSocketBuilder(*Settings.Name).AsReusable().WithBroadcast().AsNonBlocking().Build();

	int32 SendSize, ReceiveSize;
	Socket->SetSendBufferSize(Settings.BufferSize, SendSize);
	Socket->SetReceiveBufferSize(Settings.BufferSize, ReceiveSize);

	if (!Socket->Connect(*Addr))
	{
		UE_LOG(LogInworldAIIntegration, Error, TEXT("FSocketSend::Initialize couldn't connect"));
		return false;
	}

	return true;
}

bool Inworld::FSocketSend::Deinitialize()
{
	if (!Socket)
	{
		return true;
	}

	const bool bSuccess = Socket->Close();
	ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(Socket);
	
	return bSuccess;
}

bool Inworld::FSocketSend::ProcessData(TArray<uint8>& Data)
{
	int32 BytesSent;
	if (!Socket->Send(Data.GetData(), Data.Num(), BytesSent))
	{
		return false;
	}

	return Data.Num() == BytesSent;
}

bool Inworld::FSocketReceive::Initialize(const FSocketSettings& Settings)
{
	FIPv4Address Addr;
	FIPv4Address::Parse(Settings.IpAddr, Addr);

	FIPv4Endpoint Endpoint(Addr, Settings.Port);

	Socket = FUdpSocketBuilder(*Settings.Name)
		.AsNonBlocking()
		.AsReusable()
		.BoundToEndpoint(Endpoint)
		.WithReceiveBufferSize(Settings.BufferSize)
		.Build();

	if (!Socket)
	{
		UE_LOG(LogInworldAIIntegration, Error, TEXT("FSocketReceive::Initialize couldn't build a socket"));
		return false;
	}

	Receiver = new FUdpSocketReceiver(Socket, FTimespan::FromMilliseconds(100), *Settings.Name);

	Receiver->OnDataReceived().BindLambda([this](const FArrayReaderPtr& DataPtr, const FIPv4Endpoint& Endpoint)
		{
			TArray<uint8> NewData;
			NewData.AddUninitialized(DataPtr->TotalSize());
			DataPtr->Serialize(NewData.GetData(), DataPtr->TotalSize());
			DataQueue.Enqueue(NewData);
		});

	Receiver->Start();

	return true;
}

bool Inworld::FSocketReceive::Deinitialize()
{
	if (Receiver)
	{
		Receiver->Stop();
		delete Receiver;
		Receiver = nullptr;
	}

	if (!Socket)
	{
		return true;
	}

	const bool bSuccess = Socket->Close();
	ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(Socket);
	Socket = nullptr;

	return true;
}

bool Inworld::FSocketReceive::ProcessData(TArray<uint8>& Data)
{
	return DataQueue.Dequeue(Data);
}
