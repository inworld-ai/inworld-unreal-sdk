// Copyright 2023 Theai, Inc. (DBA Inworld) All Rights Reserved.

#include "InworldCharacterPlaybackControl.h"

void UInworldCharacterPlaybackControl::OnCharacterInteractionEnd_Implementation(const FCharacterMessageInteractionEnd& Message)
{
	OnInteractionEnd.Broadcast();
}
