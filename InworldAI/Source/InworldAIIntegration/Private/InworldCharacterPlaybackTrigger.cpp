/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldCharacterPlaybackTrigger.h"
#include "InworldApi.h"

void UInworldCharacterPlaybackTrigger::OnCharacterTrigger_Implementation(const FCharacterMessageTrigger& Message)
{
	TriggerName = Message.Name;
	TriggerId = Message.InteractionId;
}

void UInworldCharacterPlaybackTrigger::OnCharacterInteractionEnd_Implementation(const FCharacterMessageInteractionEnd& Message)
{
	if (TriggerId == Message.InteractionId)
	{
		OwnerActor->GetWorld()->GetSubsystem<UInworldApiSubsystem>()->NotifyCustomTrigger(TriggerName);
	}
}
