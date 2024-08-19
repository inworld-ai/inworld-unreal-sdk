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
namespace Inworld { class FSocketBase; }

UCLASS()
class INWORLDAIINTEGRATION_API UInworldAudioRepl : public UObject, public FTickableGameObject
{
	GENERATED_BODY()
	
public:
	virtual void PostLoad() override;
	virtual void BeginDestroy() override;

	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	
	void ReplicateAudioEvent(FInworldAudioDataEvent& Event);

	// Specify port for port forwarding
	void SetPort(int32 InPort) { Port = InPort; }

private:
	void ListenAudioSocket();
	int32 GetPort(const FInternetAddr& IpAddr);

	Inworld::FSocketBase& GetAudioSocket(const TSharedPtr<FInternetAddr>& LocalAddr, const TSharedPtr<FInternetAddr>& RemoteAddr);

	TMap<FString, TUniquePtr<Inworld::FSocketBase>> AudioSockets;

	int32 Port = 0;
};
