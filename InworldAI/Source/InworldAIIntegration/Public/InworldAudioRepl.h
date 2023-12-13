// Copyright 2023 Theai, Inc. (DBA Inworld) All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InworldSockets.h"
#include "IPAddress.h"

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

private:
	void ListenAudioSocket();

	Inworld::FSocketBase& GetAudioSocket(const FInternetAddr& IpAddr);

	TMap<FString, TUniquePtr<Inworld::FSocketBase>> AudioSockets;
};
