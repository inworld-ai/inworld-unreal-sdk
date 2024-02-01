/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldCharacterPlaybackTrigger.h"
#include "InworldApi.h"

void UInworldCharacterPlaybackTrigger::OnCharacterTrigger_Implementation(const FCharacterMessageTrigger& Message)
{
	if (!PendingTriggers.Contains(Message.InteractionId))
	{
		PendingTriggers.Add(Message.InteractionId, {});
	}
	PendingTriggers[Message.InteractionId].Add(Message);
}

void UInworldCharacterPlaybackTrigger::OnCharacterInteractionEnd_Implementation(const FCharacterMessageInteractionEnd& Message)
{
	if (PendingTriggers.Contains(Message.InteractionId))
	{
		for (auto& Trigger : PendingTriggers[Message.InteractionId])
		{
			OwnerActor->GetWorld()->GetSubsystem<UInworldApiSubsystem>()->NotifyCustomTrigger(Trigger.Name);
		}
	}
}
