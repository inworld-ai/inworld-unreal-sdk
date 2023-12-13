// Copyright 2023 Theai, Inc. (DBA Inworld) All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InworldPackets.h"

namespace Inworld
{
	class IPlayerComponent;

	class ICharacterComponent
	{
	public:
		virtual void Possess(const FInworldAgentInfo& AgentInfo) = 0;
		virtual void Unpossess() = 0;
		virtual const FString& GetAgentId() const = 0;
		virtual const FString& GetGivenName() const = 0;
		virtual const FString& GetBrainName() const = 0;
		virtual void HandlePacket(TSharedPtr<FInworldPacket> Packet) = 0;
		virtual AActor* GetComponentOwner() const = 0;
		virtual IPlayerComponent* GetTargetPlayer() = 0;
	};

	class IPlayerComponent
	{
	public:
		virtual ICharacterComponent* GetTargetCharacter() = 0;
	};
}
