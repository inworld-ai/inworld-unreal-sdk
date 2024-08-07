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

	TSharedPtr<FCharacterMessageUtteranceDataInworld> UtteranceData = StaticCastSharedPtr<FCharacterMessageUtteranceDataInworld>(Message.UtteranceData);

	if (!UtteranceData)
	{
		Message.UtteranceData = UtteranceData = MakeShared<FCharacterMessageUtteranceDataInworld>();
	}

	UtteranceData->SoundData.Append(Event.Chunk);

	FWaveModInfo WaveInfo;
	if (WaveInfo.ReadWaveInfo(UtteranceData->SoundData.GetData(), UtteranceData->SoundData.Num()))
	{
		UtteranceData->ChannelCount = *WaveInfo.pChannels;
		UtteranceData->SamplesPerSecond = *WaveInfo.pSamplesPerSec;
		UtteranceData->BitsPerSample = *WaveInfo.pBitsPerSample;
	}

	ensure(!UtteranceData->bAudioFinal);
	UtteranceData->bAudioFinal = Event.bFinal;

	auto& InworldVisemeInfos = Event.VisemeInfos;
	UtteranceData->VisemeInfos.Reserve(UtteranceData->VisemeInfos.Num() + InworldVisemeInfos.Num());
	if (UtteranceData->VisemeInfos.Num() == 0)
	{
		UtteranceData->VisemeInfos.Add({ TEXT("STOP"), 0.f });
	}
	for (auto& VisemeInfo : InworldVisemeInfos)
	{
		if (!VisemeInfo.Code.IsEmpty())
		{
			UtteranceData->VisemeInfos.Add({ VisemeInfo.Code, VisemeInfo.Timestamp });
		}
	}
	if (UtteranceData->bAudioFinal)
	{
		const float Duration = *WaveInfo.pWaveDataSize / (*WaveInfo.pChannels * (*WaveInfo.pBitsPerSample / 8.f) * *WaveInfo.pSamplesPerSec);
		UtteranceData->VisemeInfos.Add({ TEXT("STOP"), Duration });
	}
}

void operator<<(FCharacterMessageUtterance& Message, const FInworldA2FHeaderEvent& Event)
{
	((FCharacterMessage&)(Message)) << Event;

	TSharedPtr<FCharacterMessageUtteranceDataA2F> UtteranceData = StaticCastSharedPtr<FCharacterMessageUtteranceDataA2F>(Message.UtteranceData);

	if (!UtteranceData)
	{
		Message.UtteranceData = UtteranceData = MakeShared<FCharacterMessageUtteranceDataA2F>();
	}
	
	if (Event.ChannelCount == 0 && Event.SamplesPerSecond == 0 && Event.BitsPerSample == 0)
	{
		if (!UtteranceData->bRecvEnd)
		{
			UtteranceData->bRecvEnd = true;
		}
		else
		{
			UtteranceData->bAudioFinal = true;
		}
	}
	else
	{
		UtteranceData->ChannelCount = Event.ChannelCount;
		UtteranceData->SamplesPerSecond = Event.SamplesPerSecond;
		UtteranceData->BitsPerSample = Event.BitsPerSample;
		UtteranceData->BlendShapeNames = Event.BlendShapes;
	}
}

void operator<<(FCharacterMessageUtterance& Message, const FInworldA2FContentEvent& Event)
{
	((FCharacterMessage&)(Message)) << Event;

	TSharedPtr<FCharacterMessageUtteranceDataA2F> UtteranceData = StaticCastSharedPtr<FCharacterMessageUtteranceDataA2F>(Message.UtteranceData);

	ensure(UtteranceData);
	UtteranceData->SoundData.Append(Event.AudioInfo.Audio);

	TMap<FName, float> BlendShapeMap;
	for (int32 i = 0; i < UtteranceData->BlendShapeNames.Num(); ++i)
	{
		BlendShapeMap.Add(UtteranceData->BlendShapeNames[i], Event.BlendShapeWeights.Values[i]);
	}
	UtteranceData->BlendShapeMaps.Add(BlendShapeMap);
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

template<>
bool FCharacterMessageUtteranceData::IsType<FCharacterMessageUtteranceDataInworld>() { return Type == ECharacterMessageUtteranceDataType::INWORLD; }
template<>
bool FCharacterMessageUtteranceData::IsType<FCharacterMessageUtteranceDataA2F>() { return Type == ECharacterMessageUtteranceDataType::A2F; }
