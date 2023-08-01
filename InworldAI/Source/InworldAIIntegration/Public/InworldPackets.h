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

void AppendToDebugString(FString& DbgStr, const FString& Str);

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

	void AppendDebugString(FString& Str) const;

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

	void Serialize(FMemoryArchive& Ar);

	void AppendDebugString(FString& Str) const;

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

	void AppendDebugString(FString& Str) const;

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

	FString ToDebugString() const;

	UPROPERTY()
	FInworldPacketId PacketId;
	UPROPERTY()
	FInworldRouting Routing;

protected:
	virtual void AppendDebugString(FString& Str) const PURE_VIRTUAL(FInworldPacket::AppendDebugString);
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

protected:
	virtual void AppendDebugString(FString& Str) const override;
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

protected:
	virtual void AppendDebugString(FString& Str) const;
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

protected:
	virtual void AppendDebugString(FString& Str) const;
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

protected:
	virtual void AppendDebugString(FString& Str) const;
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

protected:
	virtual void AppendDebugString(FString& Str) const;
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

protected:
	virtual void AppendDebugString(FString& Str) const;
};

USTRUCT()
struct FInworldCustomEvent : public FInworldPacket
{
	GENERATED_BODY()

	FInworldCustomEvent() = default;
	FInworldCustomEvent(const Inworld::CustomEvent& Event);
	virtual ~FInworldCustomEvent() = default;

	virtual void Accept(InworldPacketVisitor & Visitor) override { Visitor.Visit(*this); }
		
	UPROPERTY()
	FString Name;

	UPROPERTY(NotReplicated)
	TMap<FString, FString> Params;
	
	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		TArray<FString> ParamKeys;
		TArray<FString> ParamValues;

		if (Ar.IsLoading())
		{
			Ar << ParamKeys;
			Ar << ParamValues;
			for (auto It = ParamKeys.CreateConstIterator(); It; ++It)
			{
				Params.Add(ParamKeys[It.GetIndex()], ParamValues[It.GetIndex()]);
			}
		}
		else
		{
			Params.GenerateKeyArray(ParamKeys);
			Params.GenerateValueArray(ParamValues);
			Ar << ParamKeys;
			Ar << ParamValues;
		}

		bOutSuccess = true;
		return true;
	}
	
protected:
	virtual void AppendDebugString(FString& Str) const;
};

template<>
struct TStructOpsTypeTraits<FInworldCustomEvent> : public TStructOpsTypeTraitsBase2<FInworldCustomEvent>
{
	enum
	{
		WithNetSerializer = true
	};
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

protected:
	virtual void AppendDebugString(FString& Str) const;
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

protected:
	template<typename TNew, typename TOrig>
	void Create(const TOrig& Event)
	{
		Packet = std::make_shared<TNew>(Event);
	}

	std::shared_ptr<FInworldPacket> Packet;
};