/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldCharacterMessage.h"

void operator<<(FCharacterMessage& Message, const FInworldPacket& Packet)
{
	Message.UtteranceId = Packet.PacketId.UtteranceId;
	Message.InteractionId = Packet.PacketId.InteractionId;
}

void operator<<(FCharacterMessageUtterance& Message, const FInworldTextEvent& Event)
{
	((FCharacterMessage&)(Message)) << Event;
	Message.Text = Event.Text;
	Message.bTextFinal = Event.Final;
}

void operator<<(FCharacterMessageUtterance& Message, const FInworldAudioDataEvent& Event)
{
	((FCharacterMessage&)(Message)) << Event;
	Message.SoundData.Append(Event.Chunk);

	ensure(!Message.bAudioFinal);
	Message.bAudioFinal = Event.bFinal;

	auto& InworldVisemeInfos = Event.VisemeInfos;
	Message.VisemeInfos.Reserve(InworldVisemeInfos.Num());
	for (auto& VisemeInfo : InworldVisemeInfos)
	{
		FCharacterUtteranceVisemeInfo& VisemeInfo_Ref = Message.VisemeInfos.AddDefaulted_GetRef();
		VisemeInfo_Ref.Timestamp = VisemeInfo.Timestamp;
		VisemeInfo_Ref.Code = VisemeInfo.Code;
	}
}

void operator<<(FCharacterMessagePlayerTalk& Message, const FInworldTextEvent& Event)
{
	((FCharacterMessage&)(Message)) << Event;
	Message.Text = Event.Text;
	Message.bTextFinal = Event.Final;
}

void operator<<(FCharacterMessageSilence& Message, const FInworldSilenceEvent& Event)
{
	((FCharacterMessage&)(Message)) << Event;
	Message.Duration = Event.Duration;
}

void operator<<(FCharacterMessageTrigger& Message, const FInworldCustomEvent& Event)
{
	((FCharacterMessage&)(Message)) << Event;
	Message.Name = Event.Name;
	Message.Params = Event.Params.RepMap;
}

void operator<<(FCharacterMessageTrigger& Message, const FInworldRelationEvent& Event)
{
	((FCharacterMessage&)(Message)) << Event;
	Message.Name = TEXT("inworld.relation.update");
	Message.Params.Add(TEXT("Attraction"), FString::FromInt(Event.Attraction));
	Message.Params.Add(TEXT("Familiar"), FString::FromInt(Event.Familiar));
	Message.Params.Add(TEXT("Flirtatious"), FString::FromInt(Event.Flirtatious));
	Message.Params.Add(TEXT("Respect"), FString::FromInt(Event.Respect));
	Message.Params.Add(TEXT("Trust"), FString::FromInt(Event.Trust));
}

void operator<<(FCharacterMessageInteractionEnd& Message, const FInworldControlEvent& Event)
{
	((FCharacterMessage&)(Message)) << Event;
}
