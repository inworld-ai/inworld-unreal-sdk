/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "Packets.h"
#include "proto/ProtoDisableWarning.h"

#include <random>

namespace Inworld {

    std::string RandomUUID()
    {
        const std::string Symbols = "0123456789abcdef";

        std::string Result = "00000000-0000-0000-0000-0000000000";

		std::random_device Rd;
		std::mt19937 Gen(Rd());
		std::uniform_int_distribution<> Distr(0, Symbols.size() - 1);

        for (int i = 0; i < Result.size(); i++) {
            if (Result[i] != '-')
            {
				Result[i] = Symbols[Distr(Gen)];
            }
        }
        return Result;
    }

    InworldPakets::Actor Actor::ToProto() const
    {
        InworldPakets::Actor actor;
        actor.set_type(_Type);
        actor.set_name(_Name);
        return actor;
    }

    InworldPakets::Routing Routing::ToProto() const
    {
        InworldPakets::Routing routing;
        *routing.mutable_source() = _Source.ToProto();
        *routing.mutable_target() = _Target.ToProto();
        return routing;
    }

    Routing Routing::Player2Agent(const std::string& AgentId) {
        return Routing(Actor(InworldPakets::Actor_Type_PLAYER, ""), Actor(InworldPakets::Actor_Type_AGENT, AgentId));
    }

    InworldPakets::PacketId PacketId::ToProto() const
    {
        InworldPakets::PacketId proto;
        proto.set_packet_id(_UID);
        proto.set_utterance_id(_UtteranceId);
        proto.set_interaction_id(_InteractionId);
        return proto;
    }

    InworldPakets::InworldPacket Packet::ToProto() const
    {
        InworldPakets::InworldPacket Proto;
        *Proto.mutable_packet_id() = _PacketId.ToProto();
        *Proto.mutable_routing() = _Routing.ToProto();
        *Proto.mutable_timestamp() = 
            ::google::protobuf::util::TimeUtil::TimeTToTimestamp(std::chrono::duration_cast<std::chrono::seconds>(_Timestamp.time_since_epoch()).count());
        ToProtoInternal(Proto);
        return Proto;
    }

    void TextEvent::ToProtoInternal(InworldPakets::InworldPacket& Proto) const 
    {
        Proto.mutable_text()->set_text(_Text);
        Proto.mutable_text()->set_final(_Final);
        Proto.mutable_text()->set_source_type(_SourceType);
    }

    void ControlEvent::ToProtoInternal(InworldPakets::InworldPacket& Proto) const
    {
        Proto.mutable_control()->set_action(_Action);
    }

    void DataEvent::ToProtoInternal(InworldPakets::InworldPacket& Proto) const
    {
        Proto.mutable_data_chunk()->set_chunk(_Chunk);
    }

    void AudioDataEvent::ToProtoInternal(InworldPakets::InworldPacket& Proto) const
    {
        DataEvent::ToProtoInternal(Proto);
        Proto.mutable_data_chunk()->set_type(GetType());

        for (const auto& phoneme_info : GetPhonemeInfos())
        {
            auto* info = Proto.mutable_data_chunk()->add_additional_phoneme_info();
            info->set_phoneme(phoneme_info.Code);
            info->mutable_start_offset()->set_seconds(phoneme_info.Timestamp);
            info->mutable_start_offset()->set_nanos(
                (phoneme_info.Timestamp - std::floor(phoneme_info.Timestamp)) * 1000000000);
        }
    }

    AudioDataEvent::AudioDataEvent(const InworldPakets::InworldPacket& GrpcPacket) : DataEvent(GrpcPacket)
    {
        _PhonemeInfos.reserve(GrpcPacket.data_chunk().additional_phoneme_info_size());
        for (const auto& phoneme_info : GrpcPacket.data_chunk().additional_phoneme_info())
        {
            _PhonemeInfos.emplace_back();
            PhonemeInfo& back = _PhonemeInfos.back();
            back.Code = phoneme_info.phoneme();
            back.Timestamp = phoneme_info.start_offset().seconds() + (phoneme_info.start_offset().nanos() / 1000000000.f);
        }
    }

    EmotionEvent::EmotionEvent(const InworldPakets::InworldPacket& GrpcPacket) : Packet(GrpcPacket)
    {
        _Behavior = GrpcPacket.emotion().behavior();
        _Strength = GrpcPacket.emotion().strength();
    }

    void EmotionEvent::ToProtoInternal(InworldPakets::InworldPacket& Proto) const
    {
        Proto.mutable_emotion()->set_behavior(_Behavior);
    }

	void CustomGestureEvent::ToProtoInternal(InworldPakets::InworldPacket& Proto) const
	{
		
	}

    CustomEvent::CustomEvent(const InworldPakets::InworldPacket& GrpcPacket) : Packet(GrpcPacket)
    {
        _Name = GrpcPacket.custom().name().data();
        for(const auto& Param : GrpcPacket.custom().parameters())
        {
            _Params.insert(std::make_pair<std::string, std::string>(Param.name().data(), Param.value().data()));
        }
    }

	void CustomEvent::ToProtoInternal(InworldPakets::InworldPacket& Proto) const
	{
        auto* mutable_custom = Proto.mutable_custom();
        mutable_custom->set_name(_Name);
        for (const std::pair<const std::string, std::string>& Param : _Params)
        {
            auto* param = mutable_custom->add_parameters();
            param->set_name(Param.first);
            param->set_value(Param.second);
        }
	}

	void SilenceEvent::ToProtoInternal(InworldPakets::InworldPacket& Proto) const
	{
    
	}

    void CancelResponseEvent::ToProtoInternal(InworldPakets::InworldPacket& Proto) const
    {
        auto* mutable_cancel_responses = Proto.mutable_mutation()->mutable_cancel_responses();
        mutable_cancel_responses->set_interaction_id(_InteractionId);
        for (const auto& UtteranceId : _UtteranceIds)
        {
            mutable_cancel_responses->add_utterance_id(UtteranceId);
        }
    }

    ChangeSceneEvent::ChangeSceneEvent(const InworldPakets::InworldPacket& GrpcPacket) : MutationEvent(GrpcPacket)
    {
        _AgentInfos.reserve(GrpcPacket.load_scene_output().agents_size());
        for (const auto& agent : GrpcPacket.load_scene_output().agents())
        {
            _AgentInfos.emplace_back();
            Inworld::AgentInfo& back = _AgentInfos.back();
            back.AgentId = agent.agent_id();
            back.BrainName = agent.brain_name();
            back.GivenName = agent.given_name();
        }
    }

    void ChangeSceneEvent::ToProtoInternal(InworldPakets::InworldPacket& Proto) const
    {
        auto* mutable_load_scene = Proto.mutable_mutation()->mutable_load_scene();
        mutable_load_scene->set_name(_SceneName);
    }

}
