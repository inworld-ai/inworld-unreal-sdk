// Copyright 2023 Theai, Inc. (DBA Inworld) All Rights Reserved.

#include "InworldCharacterPlaybackText.h"
#include "InworldCharacterMessage.h"

UInworldCharacterPlaybackText::FInworldCharacterText::FInworldCharacterText(const FString& InUtteranceId, const FString& InText, bool bInTextFinal, bool bInIsPlayer)
	: Id(InUtteranceId)
	, Text(InText)
	, bTextFinal(bInTextFinal)
	, bIsPlayer(bInIsPlayer)
{}

void UInworldCharacterPlaybackText::OnCharacterUtterance_Implementation(const FCharacterMessageUtterance& Message)
{
	UpdateUtterance(Message.InteractionId, Message.UtteranceId, Message.Text, Message.bTextFinal, false);
}

void UInworldCharacterPlaybackText::OnCharacterUtteranceInterrupt_Implementation(const FCharacterMessageUtterance& Message)
{
	auto* CharacterText = CharacterTexts.FindByPredicate([Message](const auto& Text) { return Text.Id == Message.UtteranceId; });
	if(CharacterText)
	{
		OnCharacterTextInterrupt.Broadcast(CharacterText->Id);
	}
}

void UInworldCharacterPlaybackText::OnCharacterPlayerTalk_Implementation(const FCharacterMessagePlayerTalk& Message)
{
	UpdateUtterance(Message.InteractionId, Message.UtteranceId, Message.Text, Message.bTextFinal, true);
}

void UInworldCharacterPlaybackText::OnCharacterInteractionEnd_Implementation(const FCharacterMessageInteractionEnd& Message)
{
	const FString InteractionId = Message.InteractionId;
	if (InteractionIdToUtteranceIdMap.Contains(InteractionId))
	{
		const TArray<FString>& InteractionUtteranceIds = InteractionIdToUtteranceIdMap[InteractionId];
		CharacterTexts.RemoveAll([InteractionUtteranceIds](const auto& Text) { return InteractionUtteranceIds.Contains(Text.Id); });
		InteractionIdToUtteranceIdMap.Remove(InteractionId);
	}
}

void UInworldCharacterPlaybackText::UpdateUtterance(const FString& InteractionId, const FString& UtteranceId, const FString& Text, bool bTextFinal, bool bIsPlayer)
{
	if (!InteractionIdToUtteranceIdMap.Contains(InteractionId))
	{
		InteractionIdToUtteranceIdMap.Add(InteractionId, TArray<FString>());
	}

	auto* CharacterText = CharacterTexts.FindByPredicate([UtteranceId](const auto& Text) { return Text.Id == UtteranceId; });
	if (CharacterText)
	{
		*CharacterText = FInworldCharacterText(UtteranceId, Text, bTextFinal, bIsPlayer);
	}
	else
	{
		CharacterText = &CharacterTexts.Emplace_GetRef(UtteranceId, Text, bTextFinal, bIsPlayer);
		InteractionIdToUtteranceIdMap[InteractionId].Add(CharacterText->Id);
		OnCharacterTextStart.Broadcast(CharacterText->Id, CharacterText->bIsPlayer);
	}

	OnCharacterTextChanged.Broadcast(CharacterText->Id, CharacterText->bIsPlayer, CharacterText->Text);
	if (CharacterText->bTextFinal)
	{
		OnCharacterTextFinal.Broadcast(CharacterText->Id, CharacterText->bIsPlayer, CharacterText->Text);
	}
}
