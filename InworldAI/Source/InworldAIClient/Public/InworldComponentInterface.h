/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "InworldState.h"
#include "InworldPackets.h"

namespace Inworld
{
	class IPlayerComponent;

	class ICharacterComponent
	{
	public:
		virtual void Possess(const Inworld::AgentInfo& AgentInfo) = 0;
		virtual void Unpossess() = 0;
		virtual const FString& GetAgentId() const = 0;
		virtual const FString& GetGivenName() const = 0;
		virtual const FString& GetBrainName() const = 0;
		virtual void HandlePacket(std::shared_ptr<FInworldPacket> Packet) = 0;
		virtual AActor* GetComponentOwner() const = 0;
		virtual IPlayerComponent* GetTargetPlayer() = 0;
	};

	class IPlayerComponent
	{
	public:
		virtual ICharacterComponent* GetTargetCharacter() = 0;
	};
}
