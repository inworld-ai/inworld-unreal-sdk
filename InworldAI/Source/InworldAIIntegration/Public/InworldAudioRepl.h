/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "InworldSockets.h"
#include "IPAddress.h"
#include "Tickable.h"

#include "InworldAudioRepl.generated.h"

struct FInworldAudioDataEvent;
class UNetConnection;
namespace Inworld { class FSocketBase; }

UCLASS()
class INWORLDAIINTEGRATION_API UInworldAudioRepl : public UObject, public FTickableGameObject
{
	GENERATED_BODY()
	
public:
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	
	void ReplicateAudioEvent(const FInworldAudioDataEvent& Event);

	// Specify port for port forwarding
	void SetPort(int32 InPort) { Port = InPort; }

private:
	void ListenAudioSocket();
	TSharedPtr<FInternetAddr> CreateIpAddr(const TSharedPtr<FInternetAddr>& IpAddr);

	Inworld::FSocketBase& GetAudioSocket(const UNetConnection& Connection);

	TMap<FString, TUniquePtr<Inworld::FSocketBase>> AudioSockets;

	int32 Port = 0;
};
