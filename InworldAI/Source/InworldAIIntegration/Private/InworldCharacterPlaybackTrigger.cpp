// Copyright 2023 Theai, Inc. (DBA Inworld) All Rights Reserved.

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
