/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "NDK/Packets.h"
#include "InworldEnums.h"
#include "InworldUtils.h"

#include "InworldPackets.generated.h"

USTRUCT()
struct FInworldActor
{
	GENERATED_BODY()

	FInworldActor() = default;
	FInworldActor(const FInworldActor& Other) = default;
	FInworldActor(const Inworld::Actor& Actor)
		: Type(static_cast<EInworldActorType>(Actor._Type))
		, Name(UTF8_TO_TCHAR(Actor._Name.c_str()))
	{}

	void Serialize(FMemoryArchive& Ar);

	UPROPERTY()
	EInworldActorType Type =  EInworldActorType::UNKNOWN;
	UPROPERTY()
	FString Name;
};

USTRUCT()
struct FInworldRouting
{
	GENERATED_BODY()
	
	FInworldRouting() = default;
	FInworldRouting(const FInworldRouting& Other) = default;
	FInworldRouting(const Inworld::Routing& Routing)
		: Source(Routing._Source)
		, Target(Routing._Target)
	{}

	void Serialize(FMemoryArchive & Ar);

	UPROPERTY()
	FInworldActor Source;
	UPROPERTY()
	FInworldActor Target;
};

USTRUCT()
struct FInworldPacketId
{
	GENERATED_BODY()

	FInworldPacketId() = default;
	FInworldPacketId(const FInworldPacketId& Other) = default;
	FInworldPacketId(const Inworld::PacketId& PacketId)
		: UID(UTF8_TO_TCHAR(PacketId._UID.c_str()))
		, UtteranceId(UTF8_TO_TCHAR(PacketId._UtteranceId.c_str()))
		, InteractionId(UTF8_TO_TCHAR(PacketId._InteractionId.c_str()))
	{}

	void Serialize(FMemoryArchive & Ar);

	UPROPERTY()
	FString UID;
	UPROPERTY()
	FString UtteranceId;
	UPROPERTY()
	FString InteractionId;
};

struct FInworldTextEvent;
struct FInworldDataEvent;
struct FInworldAudioDataEvent;
struct FInworldSilenceEvent;
struct FInworldControlEvent;
struct FInworldEmotionEvent;
struct FInworldCancelResponseEvent;
struct FInworldSimpleGestureEvent;
struct FInworldCustomGestureEvent;
struct FInworldCustomEvent;
struct FInworldChangeSceneEvent;

class InworldPacketVisitor
{
public:
	virtual void Visit(const FInworldTextEvent& Event) {  }
	virtual void Visit(const FInworldDataEvent& Event) {  }
	virtual void Visit(const FInworldAudioDataEvent& Event) {  }
	virtual void Visit(const FInworldSilenceEvent& Event) {  }
	virtual void Visit(const FInworldControlEvent& Event) {  }
	virtual void Visit(const FInworldEmotionEvent& Event) {  }
	virtual void Visit(const FInworldCancelResponseEvent& Event) {  }
	virtual void Visit(const FInworldSimpleGestureEvent& Event) {  }
	virtual void Visit(const FInworldCustomGestureEvent& Event) {  }
	virtual void Visit(const FInworldCustomEvent& Event) {  }
	virtual void Visit(const FInworldChangeSceneEvent& Event) {  }
};

USTRUCT()
struct FInworldPacket
{
	GENERATED_BODY()

	FInworldPacket() = default;
	FInworldPacket(const Inworld::Packet& Packet)
		: PacketId(Packet._PacketId)
		, Routing(Packet._Routing)
	{}
	virtual ~FInworldPacket() = default;

	virtual void Accept(InworldPacketVisitor& Visitor) {}

	virtual void Serialize(FMemoryArchive& Ar);

	UPROPERTY()
	FInworldPacketId PacketId;
	UPROPERTY()
	FInworldRouting Routing;
};

USTRUCT()
struct FInworldTextEvent : public FInworldPacket
{
	GENERATED_BODY()

	FInworldTextEvent() = default;
	FInworldTextEvent(const Inworld::TextEvent& Event)
		: FInworldPacket(Event)
		, Text(UTF8_TO_TCHAR(Event.GetText().c_str()))
		, Final(Event.IsFinal())
	{}
	virtual ~FInworldTextEvent() = default;

	virtual void Accept(InworldPacketVisitor& Visitor) override { Visitor.Visit(*this); }

	UPROPERTY()
	FString Text;
	UPROPERTY()
	bool Final = false;
};

struct FInworldDataEvent : public FInworldPacket
{
	FInworldDataEvent() = default;
	FInworldDataEvent(const Inworld::DataEvent& Event)
		: FInworldPacket(Event)
		, Chunk(Event.GetDataChunk())
	{
	}
	virtual ~FInworldDataEvent() = default;

	virtual void Accept(InworldPacketVisitor & Visitor) override { Visitor.Visit(*this); }

	virtual void Serialize(FMemoryArchive& Ar) override;

	std::string Chunk;
};

struct FInworldPhonemeInfo
{
	FInworldPhonemeInfo() = default;
	FInworldPhonemeInfo(const Inworld::AudioDataEvent::PhonemeInfo& Info)
		: Code(Info.Code)
		, Timestamp(Info.Timestamp)
	{}

	void Serialize(FMemoryArchive& Ar);

	std::string Code;
	float Timestamp;
};

struct FInworldAudioDataEvent : public FInworldDataEvent
{
	FInworldAudioDataEvent() = default;
	FInworldAudioDataEvent(const Inworld::AudioDataEvent& Event);
	virtual ~FInworldAudioDataEvent() = default;

	virtual void Accept(InworldPacketVisitor& Visitor) override { Visitor.Visit(*this); }

	static void ConvertToReplicatableEvents(const FInworldAudioDataEvent& Event, TArray<FInworldAudioDataEvent>& RepEvents);

	virtual void Serialize(FMemoryArchive& Ar) override;

	TArray<FInworldPhonemeInfo> PhonemeInfos;
	bool bFinal = true;
};

USTRUCT()
struct FInworldSilenceEvent : public FInworldPacket
{
	GENERATED_BODY()

	FInworldSilenceEvent() = default;
	FInworldSilenceEvent(const Inworld::SilenceEvent& Event)
		: FInworldPacket(Event)
		, Duration(Event.GetDuration())
	{}
	virtual ~FInworldSilenceEvent() = default;

	virtual void Accept(InworldPacketVisitor & Visitor) override { Visitor.Visit(*this); }

	UPROPERTY()
	float Duration = 0.f;
};

USTRUCT()
struct FInworldControlEvent : public FInworldPacket
{
	GENERATED_BODY()

	FInworldControlEvent() = default;
	FInworldControlEvent(const Inworld::ControlEvent& Event)
		: FInworldPacket(Event)
		, Action(static_cast<EInworldControlEventAction>(Event.GetControlAction()))
	{}
	virtual ~FInworldControlEvent() = default;

	virtual void Accept(InworldPacketVisitor & Visitor) override { Visitor.Visit(*this); }

	UPROPERTY()
	EInworldControlEventAction Action = EInworldControlEventAction::UNKNOWN;
};

USTRUCT()
struct FInworldEmotionEvent : public FInworldPacket
{
	GENERATED_BODY()

	FInworldEmotionEvent() = default;
	FInworldEmotionEvent(const Inworld::EmotionEvent& Event)
		: FInworldPacket(Event)
		, Behavior(static_cast<EInworldCharacterEmotionalBehavior>(Event.GetEmotionalBehavior()))
		, Strength(static_cast<EInworldCharacterEmotionStrength>(Event.GetStrength()))
	{}
	virtual ~FInworldEmotionEvent() = default;

	virtual void Accept(InworldPacketVisitor & Visitor) override { Visitor.Visit(*this); }
	
	UPROPERTY()
	EInworldCharacterEmotionalBehavior Behavior = EInworldCharacterEmotionalBehavior::NEUTRAL;
	UPROPERTY()
	EInworldCharacterEmotionStrength Strength = EInworldCharacterEmotionStrength::NORMAL;
};

USTRUCT()
struct FInworldCustomEvent : public FInworldPacket
{
	GENERATED_BODY()

	FInworldCustomEvent() = default;
	FInworldCustomEvent(const Inworld::CustomEvent& Event)
		: FInworldPacket(Event)
		, Name(UTF8_TO_TCHAR(Event.GetName().c_str()))
	{}
	virtual ~FInworldCustomEvent() = default;

	virtual void Accept(InworldPacketVisitor & Visitor) override { Visitor.Visit(*this); }
		
	UPROPERTY()
	FString Name;
};

USTRUCT()
struct FInworldChangeSceneEvent : public FInworldPacket
{
	GENERATED_BODY()

	FInworldChangeSceneEvent() = default;
	FInworldChangeSceneEvent(const Inworld::ChangeSceneEvent& Event)
		: AgentInfos(Event.GetAgentInfos())
	{}
	virtual ~FInworldChangeSceneEvent() = default;

	virtual void Accept(InworldPacketVisitor& Visitor) override { Visitor.Visit(*this); }

	std::vector<Inworld::AgentInfo> AgentInfos;
};

class InworldPacketCreator : public Inworld::PacketVisitor
{
public:
	virtual ~InworldPacketCreator() = default;

	virtual void Visit(const Inworld::TextEvent& Event) override { Create<FInworldTextEvent, Inworld::TextEvent>(Event); }
	virtual void Visit(const Inworld::DataEvent& Event) override { Create<FInworldDataEvent, Inworld::DataEvent>(Event); }
	virtual void Visit(const Inworld::AudioDataEvent& Event) override { Create<FInworldAudioDataEvent, Inworld::AudioDataEvent>(Event); }
	virtual void Visit(const Inworld::SilenceEvent& Event) override { Create<FInworldSilenceEvent, Inworld::SilenceEvent>(Event); }
	virtual void Visit(const Inworld::ControlEvent& Event) override { Create<FInworldControlEvent, Inworld::ControlEvent>(Event); }
	virtual void Visit(const Inworld::EmotionEvent& Event) override { Create<FInworldEmotionEvent, Inworld::EmotionEvent>(Event); }
	virtual void Visit(const Inworld::CustomEvent& Event) override { Create<FInworldCustomEvent, Inworld::CustomEvent>(Event); }
	virtual void Visit(const Inworld::ChangeSceneEvent& Event) override { Create<FInworldChangeSceneEvent, Inworld::ChangeSceneEvent>(Event); }

	std::shared_ptr<FInworldPacket> GetPacket() { return Packet; }

private:
	template<typename TNew, typename TOrig>
	void Create(const TOrig& Event)
	{
		Packet = std::make_shared<TNew>(Event);
	}

	std::shared_ptr<FInworldPacket> Packet;
};