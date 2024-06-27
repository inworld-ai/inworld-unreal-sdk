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

void UInworldAudioRepl::PostLoad()
{
	Super::PostLoad();
}

void UInworldAudioRepl::BeginDestroy()
{
	for (auto& Socket : AudioSockets)
	{
		Socket.Value->Deinitialize();
		Socket.Value.Reset();
	}
	AudioSockets.Empty();
	
	Super::BeginDestroy();
}

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

void UInworldAudioRepl::ReplicateAudioEvent(FInworldAudioDataEvent& Event)
{
	auto It = GetWorld()->GetControllerIterator();
	if (!It)
	{
		return;
	}

	TArray<uint8> Data;
	FMemoryWriter Ar(Data);
	Event.Serialize(Ar);

	for (; It; ++It)
	{
		if (UNetConnection* Connection = It->Get()->GetNetConnection())
		{
			GetAudioSocket(*Connection->RemoteAddr.Get()).ProcessData(Data);
		}
	}
}

void UInworldAudioRepl::ListenAudioSocket()
{
	auto* Ctrl = GetWorld()->GetFirstPlayerController();
	if (!Ctrl)
	{
		return;
	}

	auto* Connection = Ctrl->GetNetConnection();
	if (!Connection)
	{
		return;
	}

	auto* Driver = Connection->GetDriver();
	if (!Driver)
	{
		return;
	}


	TArray<uint8> Data;
	if (!GetAudioSocket(*Driver->GetLocalAddr().Get()).ProcessData(Data))
	{
		return;
	}

	FMemoryReader Ar(Data);

	TSharedPtr<FInworldAudioDataEvent> Event = MakeShared<FInworldAudioDataEvent>();
	Event->Serialize(Ar);

	auto* InworldApi = GetWorld()->GetSubsystem<UInworldApiSubsystem>();
	if (ensure(InworldApi))
	{
		InworldApi->HandleAudioEventOnClient(Event);
	}
}

Inworld::FSocketBase& UInworldAudioRepl::GetAudioSocket(const FInternetAddr& IpAddr)
{
	const FString IpAddrStr = IpAddr.ToString(true);
	if (auto* Socket = AudioSockets.Find(IpAddrStr))
	{
		return *Socket->Get();
	}

	Inworld::FSocketSettings Settings;
	Settings.IpAddr = IpAddr.ToString(false);
	Settings.Port = FMath::Clamp(IpAddr.GetPort() - 1000, 0, 64 * 1024);
	Settings.BufferSize = 2 * 1024 * 1024;
	Settings.Name = FString::Printf(TEXT("Inworld %s"), *IpAddrStr);

	TUniquePtr<Inworld::FSocketBase> Socket;
	if (GetWorld()->GetNetMode() == NM_Client)
	{
		Socket = MakeUnique<Inworld::FSocketReceive>();
	}
	else
	{
		Socket = MakeUnique<Inworld::FSocketSend>();
	}

	Socket->Initialize(Settings);
	AudioSockets.Add(IpAddrStr, MoveTemp(Socket));

	return *AudioSockets.Find(IpAddrStr)->Get();
}
