/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldAudioRepl.h"
#include "InworldPackets.h"
#include "InworldSockets.h"
#include "InworldApi.h"

#include "Serialization/MemoryWriter.h"
#include "Serialization/MemoryReader.h"

#include <GameFramework/Controller.h>
#include <GameFramework/PlayerController.h>

#include <Engine/NetConnection.h>
#include <Engine/World.h>

#include "InworldAIIntegrationModule.h"

void UInworldAudioRepl::Tick(float DeltaTime)
{
	if (GetWorld() && GetWorld()->GetNetMode() == NM_Client)
	{
		ListenAudioSocket();
	}
}

TStatId UInworldAudioRepl::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UInworldAudioRepl, STATGROUP_Tickables);
}

void UInworldAudioRepl::ReplicateAudioEvent(const FInworldAudioDataEvent& Event)
{
	auto It = GetWorld()->GetControllerIterator();
	if (!It)
	{
		return;
	}

	TArray<uint8> Data;
	FMemoryWriter Ar(Data);
	FDateTime Time = FDateTime::UtcNow();
	Ar << Time;
	const_cast<FInworldAudioDataEvent&>(Event).Serialize(Ar);
	for (; It; ++It)
	{
		if (const UNetConnection* Connection = It->Get()->GetNetConnection())
		{
			auto& Socket = GetAudioSocket(*Connection);
			Socket.ProcessData(Data);

			if (Event.bFinal)
			{
				const auto& Settings = Socket.GetSettings();
				UE_LOG(LogInworldAIIntegration, Log, TEXT("UInworldAudioRepl::ReplicateAudioEvent sent final audio chunk from %s to %s."),
						*Settings.LocalAddr->ToString(true), *Settings.RemoteAddr->ToString(true));
			}
		}
	}
}

void UInworldAudioRepl::ListenAudioSocket()
{
	const auto* Ctrl = GetWorld()->GetFirstPlayerController();
	if (!Ctrl)
	{
		return;
	}

	const auto* Connection = Ctrl->GetNetConnection();
	if (!Connection)
	{
		return;
	}

	TArray<uint8> Data;
	auto& Socket = GetAudioSocket(*Connection);
	if (!Socket.ProcessData(Data))
	{
		return;
	}

	TSharedPtr<FInworldAudioDataEvent> Event = MakeShared<FInworldAudioDataEvent>();
	FMemoryReader Ar(Data);
	FDateTime Time;
	Ar << Time;
	Event->Serialize(Ar);

	if (Event->bFinal)
	{
		const auto Latency = FDateTime::UtcNow() - Time;
		const auto& Settings = Socket.GetSettings();
		UE_LOG(LogInworldAIIntegration, Log, TEXT("UInworldAudioRepl::ListenAudioSocket received final audio chunk from %s on %s."),
				*Settings.RemoteAddr->ToString(true), *Settings.LocalAddr->ToString(true));
		UE_LOG(LogInworldAIIntegration, Log, TEXT("UInworldAudioRepl::ListenAudioSocket latency: %d ms."),
		       static_cast<int32>(Latency.GetTotalMilliseconds()));
	}

	auto* InworldApi = GetWorld()->GetSubsystem<UInworldApiSubsystem>();
	if (ensure(InworldApi))
	{
		InworldApi->GetInworldSession()->HandleAudioEventOnClient(Event);
	}
}

TSharedPtr<FInternetAddr> UInworldAudioRepl::CreateIpAddr(const TSharedPtr<FInternetAddr>& IpAddr)
{
	const int32 NewPort = Port != 0 ? Port :
		FMath::Clamp(IpAddr->GetPort() - 1000, 0, 64 * 1024);
	auto NewAddr = IpAddr->Clone();
	NewAddr->SetPort(NewPort);
	return NewAddr;
}

Inworld::FSocketBase& UInworldAudioRepl::GetAudioSocket(const UNetConnection& Connection)
{
	const auto LocalAddr = CreateIpAddr(Connection.GetDriver()->LocalAddr);
	const auto RemoteAddr = CreateIpAddr(Connection.RemoteAddr);
	const FString Key = FString::Printf(TEXT("%s-%s"),
		*LocalAddr->ToString(true), *RemoteAddr->ToString(true));
	if (const auto* Socket = AudioSockets.Find(Key))
	{
		return *Socket->Get();
	}

	Inworld::FSocketSettings Settings;
	Settings.LocalAddr = LocalAddr;
	Settings.RemoteAddr = RemoteAddr;
	Settings.BufferSize = 2 * 1024 * 1024;
	Settings.Name = FString::Printf(TEXT("Inworld %s"), *Key);

	TUniquePtr<Inworld::FSocketBase> Socket;
	if (GetWorld()->GetNetMode() == NM_Client)
	{
		Socket = MakeUnique<Inworld::FSocketReceive>(Settings);
	}
	else
	{
		Socket = MakeUnique<Inworld::FSocketSend>(Settings);
	}

	AudioSockets.Add(Key, MoveTemp(Socket));

	return *AudioSockets.Find(Key)->Get();
}
