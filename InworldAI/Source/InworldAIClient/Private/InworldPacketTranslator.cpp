/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldPacketTranslator.h"
THIRD_PARTY_INCLUDES_START
#include "Utils/Utils.h"
THIRD_PARTY_INCLUDES_END

void InworldPacketTranslator::TranslateInworldActor(const Inworld::Actor& Original, FInworldActor& New)
{
	New.Type = static_cast<EInworldActorType>(Original._Type);
	New.Name = UTF8_TO_TCHAR(Original._Name.c_str());
}

void InworldPacketTranslator::TranslateInworldRouting(const Inworld::Routing& Original, FInworldRouting& New)
{
	TranslateInworldActor(Original._Source, New.Source);
	TranslateInworldActor(Original._Target, New.Target);

	New.ConversationId = UTF8_TO_TCHAR(Original._ConversationId.c_str());
}

void InworldPacketTranslator::TranslateInworldPacketId(const Inworld::PacketId& Original, FInworldPacketId& New)
{
	New.UID = UTF8_TO_TCHAR(Original._UID.c_str());
	New.InteractionId = UTF8_TO_TCHAR(Original._InteractionId.c_str());
	New.UtteranceId = UTF8_TO_TCHAR(Original._UtteranceId.c_str());
}

void InworldPacketTranslator::TranslateInworldPacket(const Inworld::Packet& Original, FInworldPacket& New)
{
	TranslateInworldPacketId(Original._PacketId, New.PacketId);
	TranslateInworldRouting(Original._Routing, New.Routing);
}

template<>
void InworldPacketTranslator::TranslateEvent<Inworld::TextEvent, FInworldTextEvent>(const Inworld::TextEvent& Original, FInworldTextEvent& New)
{
	TranslateInworldPacket(Original, New);
	New.Text = UTF8_TO_TCHAR(Original.GetText().c_str());
	New.Final = Original.IsFinal();
}

template<>
void InworldPacketTranslator::TranslateEvent<Inworld::DataEvent, FInworldDataEvent>(const Inworld::DataEvent& Original, FInworldDataEvent& New)
{
	TranslateInworldPacket(Original, New);
	auto& Chunk = Original.GetDataChunk();
	New.Chunk = TArray<uint8>((uint8*)Chunk.data(), Chunk.size());
}

template<>
void InworldPacketTranslator::TranslateEvent<Inworld::AudioDataEvent, FInworldAudioDataEvent>(const Inworld::AudioDataEvent& Original, FInworldAudioDataEvent& New)
{
	TranslateEvent<Inworld::DataEvent, FInworldDataEvent>(Original, New);
	New.VisemeInfos.Reserve(Original.GetPhonemeInfos().size());
	for (const auto& PhonemeInfo : Original.GetPhonemeInfos())
	{
		const FString Code = UTF8_TO_TCHAR(Inworld::Utils::PhonemeToViseme(PhonemeInfo.Code).c_str());
		if (!Code.IsEmpty())
		{
			auto& VisemeInfoRef = New.VisemeInfos.AddDefaulted_GetRef();
			VisemeInfoRef.Code = Code;
			VisemeInfoRef.Timestamp = PhonemeInfo.Timestamp;
		}
	}
}

template<>
void InworldPacketTranslator::TranslateEvent<Inworld::SilenceEvent, FInworldSilenceEvent>(const Inworld::SilenceEvent& Original, FInworldSilenceEvent& New)
{
	TranslateInworldPacket(Original, New);
	New.Duration = Original.GetDuration();
}

template<>
void InworldPacketTranslator::TranslateEvent<Inworld::ControlEvent, FInworldControlEvent>(const Inworld::ControlEvent& Original, FInworldControlEvent& New)
{
	TranslateInworldPacket(Original, New);
	New.Action = static_cast<EInworldControlEventAction>(Original.GetControlAction());
}

template <>
void InworldPacketTranslator::TranslateEvent<>(const Inworld::ControlEventConversationUpdate& Original, FInworldConversationUpdateEvent& New)
{
	TranslateInworldPacket(Original, New);
	New.EventType = static_cast<EInworldConversationUpdateType>(Original.GetType());
	New.bIncludePlayer = Original.GetIncludePlayer();
	for (const auto& Agent : Original.GetAgents())
	{
		New.Agents.Add(UTF8_TO_TCHAR(Agent.c_str()));
	}
}

template<>
void InworldPacketTranslator::TranslateEvent<Inworld::EmotionEvent, FInworldEmotionEvent>(const Inworld::EmotionEvent& Original, FInworldEmotionEvent& New)
{
	TranslateInworldPacket(Original, New);
	New.Behavior = static_cast<EInworldCharacterEmotionalBehavior>(Original.GetEmotionalBehavior());
	New.Strength = static_cast<EInworldCharacterEmotionStrength>(Original.GetStrength());
}

template<>
void InworldPacketTranslator::TranslateEvent<Inworld::CustomEvent, FInworldCustomEvent>(const Inworld::CustomEvent& Original, FInworldCustomEvent& New)
{
	TranslateInworldPacket(Original, New);
	New.Name = UTF8_TO_TCHAR(Original.GetName().c_str());
	for (const auto& Param : Original.GetParams())
	{
		New.Params.RepMap.Add(UTF8_TO_TCHAR(Param.first.c_str()), UTF8_TO_TCHAR(Param.second.c_str()));
	}
}

template<typename TOriginal, typename TNew>
void TranslateAgents(const TOriginal& Original, TNew& New)
{
	for (const auto& AgentInfo : Original.GetAgentInfos())
	{
		auto& AgentInfoRef = New.AgentInfos.AddDefaulted_GetRef();
		AgentInfoRef.BrainName = UTF8_TO_TCHAR(AgentInfo.BrainName.c_str());
		AgentInfoRef.AgentId = UTF8_TO_TCHAR(AgentInfo.AgentId.c_str());
		AgentInfoRef.GivenName = UTF8_TO_TCHAR(AgentInfo.GivenName.c_str());
	}
}

template<>
void InworldPacketTranslator::TranslateEvent<Inworld::SessionControlResponse_LoadCharacters, FInworldLoadCharactersEvent>(const Inworld::SessionControlResponse_LoadCharacters& Original, FInworldLoadCharactersEvent& New)
{
	TranslateInworldPacket(Original, New);
	TranslateAgents(Original, New);
}

template<>
void InworldPacketTranslator::TranslateEvent<Inworld::SessionControlResponse_LoadScene, FInworldChangeSceneEvent>(const Inworld::SessionControlResponse_LoadScene& Original, FInworldChangeSceneEvent& New)
{
	TranslateInworldPacket(Original, New);
	TranslateAgents(Original, New);
}

template<>
void InworldPacketTranslator::TranslateEvent<Inworld::RelationEvent, FInworldRelationEvent>(const Inworld::RelationEvent& Original, FInworldRelationEvent& New)
{
	TranslateInworldPacket(Original, New);
	New.Attraction = Original.GetAttraction();
	New.Familiar = Original.GetFamiliar();
	New.Flirtatious = Original.GetFlirtatious();
	New.Respect = Original.GetRespect();
	New.Trust = Original.GetTrust();
}
