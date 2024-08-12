/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"

#include "InworldEnums.h"
#include "InworldTypes.h"

#include "Serialization/MemoryArchive.h"

#include "InworldPackets.generated.h"

USTRUCT()
struct FInworldReplicatedMapStruct
{
	GENERATED_BODY()

	UPROPERTY(NotReplicated)
	TMap<FString, FString> RepMap;

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
				RepMap.Add(ParamKeys[It.GetIndex()], ParamValues[It.GetIndex()]);
			}
		}
		else
		{
			RepMap.GenerateKeyArray(ParamKeys);
			RepMap.GenerateValueArray(ParamValues);
			Ar << ParamKeys;
			Ar << ParamValues;
		}

		bOutSuccess = true;
		return true;
	}
};

template<>
struct TStructOpsTypeTraits<FInworldReplicatedMapStruct> : public TStructOpsTypeTraitsBase2<FInworldReplicatedMapStruct>
{
	enum
	{
		WithNetSerializer = true
	};
};

USTRUCT()
struct INWORLDAICLIENT_API FInworldActor
{
	GENERATED_BODY()

	FInworldActor() = default;
	FInworldActor(const FInworldActor& Other) = default;
	FInworldActor(const EInworldActorType& InType, const FString& InName)
		: Type(InType)
		, Name(InName)
	{}

	void Serialize(FMemoryArchive& Ar);

	void AppendDebugString(FString& Str) const;

	UPROPERTY()
	EInworldActorType Type =  EInworldActorType::UNKNOWN;
	UPROPERTY()
	FString Name;
};

USTRUCT()
struct INWORLDAICLIENT_API FInworldRouting
{
	GENERATED_BODY()
	
	FInworldRouting() = default;
	FInworldRouting(const FInworldRouting& Other) = default;
	FInworldRouting(const FInworldActor& InSource, const FInworldActor& InTarget, const FString& InConversationId)
		: Source(InSource)
		, Target(InTarget)
		, ConversationId(InConversationId)
	{}

	void Serialize(FMemoryArchive& Ar);

	void AppendDebugString(FString& Str) const;

	UPROPERTY()
	FInworldActor Source;
	UPROPERTY()
	FInworldActor Target;
	UPROPERTY()
	FString ConversationId;
};

USTRUCT()
struct INWORLDAICLIENT_API FInworldPacketId
{
	GENERATED_BODY()

	FInworldPacketId() = default;
	FInworldPacketId(const FInworldPacketId& Other) = default;
	FInworldPacketId(const FString& InUID, const FString& InUtteranceId, const FString& InInteractionId)
		: UID(InUID)
		, UtteranceId(InUtteranceId)
		, InteractionId(InInteractionId)
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
struct FInworldConversationUpdateEvent;
struct FInworldEmotionEvent;
struct FInworldCancelResponseEvent;
struct FInworldSimpleGestureEvent;
struct FInworldCustomGestureEvent;
struct FInworldCustomEvent;
struct FInworldRelationEvent;

class InworldPacketVisitor
{
public:
	virtual void Visit(const FInworldTextEvent& Event) {  }
	virtual void Visit(const FInworldDataEvent& Event) {  }
	virtual void Visit(const FInworldAudioDataEvent& Event) {  }
	virtual void Visit(const FInworldSilenceEvent& Event) {  }
	virtual void Visit(const FInworldControlEvent& Event) {  }
	virtual void Visit(const FInworldConversationUpdateEvent& Event) {  }
	virtual void Visit(const FInworldCurrentSceneStatusEvent& Event) {  }
	virtual void Visit(const FInworldEmotionEvent& Event) {  }
	virtual void Visit(const FInworldCancelResponseEvent& Event) {  }
	virtual void Visit(const FInworldSimpleGestureEvent& Event) {  }
	virtual void Visit(const FInworldCustomGestureEvent& Event) {  }
	virtual void Visit(const FInworldCustomEvent& Event) {  }
	virtual void Visit(const FInworldRelationEvent& Event) {  }
};

struct FInworldPacket;

USTRUCT(BlueprintType)
struct INWORLDAICLIENT_API FInworldWrappedPacket
{
	GENERATED_BODY()
public:
	FInworldWrappedPacket()
		: Packet(nullptr)
	{}

	FInworldWrappedPacket(TSharedPtr<FInworldPacket> InPacket)
		: Packet(InPacket)
	{}

	TSharedPtr<FInworldPacket> Packet = {};

};

USTRUCT(BlueprintType)
struct INWORLDAICLIENT_API FInworldPacket
{
	GENERATED_BODY()

	FInworldPacket() = default;
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

USTRUCT(BlueprintType)
struct INWORLDAICLIENT_API FInworldTextEvent : public FInworldPacket
{
	GENERATED_BODY()

	FInworldTextEvent() = default;
	virtual ~FInworldTextEvent() = default;

	virtual void Accept(InworldPacketVisitor& Visitor) override { Visitor.Visit(*this); }

	UPROPERTY()
	FString Text;
	UPROPERTY()
	bool Final = false;

protected:
	virtual void AppendDebugString(FString& Str) const override;
};

USTRUCT(BlueprintType)
struct INWORLDAICLIENT_API FInworldDataEvent : public FInworldPacket
{
	GENERATED_BODY()

	FInworldDataEvent() = default;
	virtual ~FInworldDataEvent() = default;

	virtual void Accept(InworldPacketVisitor & Visitor) override { Visitor.Visit(*this); }

	virtual void Serialize(FMemoryArchive& Ar) override;

	TArray<uint8> Chunk;

protected:
	virtual void AppendDebugString(FString& Str) const;
};

USTRUCT(BlueprintType)
struct INWORLDAICLIENT_API FInworldVisemeInfo
{
	GENERATED_BODY()

	FInworldVisemeInfo() = default;

	void Serialize(FMemoryArchive& Ar);

	FString Code;
	float Timestamp = 0.f;
};

USTRUCT(BlueprintType)
struct INWORLDAICLIENT_API FInworldAudioDataEvent : public FInworldDataEvent
{
	GENERATED_BODY()

	FInworldAudioDataEvent() = default;
	virtual ~FInworldAudioDataEvent() = default;

	virtual void Accept(InworldPacketVisitor& Visitor) override { Visitor.Visit(*this); }

	static void ConvertToReplicatableEvents(const FInworldAudioDataEvent& Event, TArray<FInworldAudioDataEvent>& RepEvents);

	virtual void Serialize(FMemoryArchive& Ar) override;

	TArray<FInworldVisemeInfo> VisemeInfos;
	bool bFinal = true;

protected:
	virtual void AppendDebugString(FString& Str) const override;
};

USTRUCT(BlueprintType)
struct INWORLDAICLIENT_API FInworldSilenceEvent : public FInworldPacket
{
	GENERATED_BODY()

	FInworldSilenceEvent() = default;
	virtual ~FInworldSilenceEvent() = default;

	virtual void Accept(InworldPacketVisitor & Visitor) override { Visitor.Visit(*this); }

	UPROPERTY()
	float Duration = 0.f;

protected:
	virtual void AppendDebugString(FString& Str) const override;
};

USTRUCT(BlueprintType)
struct INWORLDAICLIENT_API FInworldControlEvent : public FInworldPacket
{
	GENERATED_BODY()

	FInworldControlEvent() = default;
	virtual ~FInworldControlEvent() = default;

	virtual void Accept(InworldPacketVisitor & Visitor) override { Visitor.Visit(*this); }

	UPROPERTY()
	EInworldControlEventAction Action = EInworldControlEventAction::UNKNOWN;

	UPROPERTY()
	FString Description;

protected:
	virtual void AppendDebugString(FString& Str) const override;
};

USTRUCT()
struct INWORLDAICLIENT_API FInworldConversationUpdateEvent : public FInworldControlEvent
{
	GENERATED_BODY()

	FInworldConversationUpdateEvent() = default;
	virtual ~FInworldConversationUpdateEvent() = default;

	virtual void Accept(InworldPacketVisitor& Visitor) override { Visitor.Visit(*this); }

	UPROPERTY()
	TArray<FString> Agents;
	UPROPERTY()
	EInworldConversationUpdateType EventType;
	UPROPERTY()
	bool bIncludePlayer;

protected:
	virtual void AppendDebugString(FString& Str) const override;
};

USTRUCT()
struct INWORLDAICLIENT_API FInworldCurrentSceneStatusEvent : public FInworldControlEvent
{
	GENERATED_BODY()

	FInworldCurrentSceneStatusEvent() = default;
	virtual ~FInworldCurrentSceneStatusEvent() = default;

	virtual void Accept(InworldPacketVisitor& Visitor) override { Visitor.Visit(*this); }

	FString SceneName;
	FString SceneDescription;
	FString SceneDisplayName;

	TArray<FInworldAgentInfo> AgentInfos;

protected:
	virtual void AppendDebugString(FString& Str) const override;
};

USTRUCT(BlueprintType)
struct INWORLDAICLIENT_API FInworldEmotionEvent : public FInworldPacket
{
	GENERATED_BODY()

	FInworldEmotionEvent() = default;
	virtual ~FInworldEmotionEvent() = default;

	virtual void Accept(InworldPacketVisitor & Visitor) override { Visitor.Visit(*this); }
	
	UPROPERTY()
	EInworldCharacterEmotionalBehavior Behavior = EInworldCharacterEmotionalBehavior::NEUTRAL;
	UPROPERTY()
	EInworldCharacterEmotionStrength Strength = EInworldCharacterEmotionStrength::NORMAL;

protected:
	virtual void AppendDebugString(FString& Str) const override;
};

USTRUCT(BlueprintType)
struct INWORLDAICLIENT_API FInworldCustomEvent : public FInworldPacket
{
	GENERATED_BODY()

	FInworldCustomEvent() = default;
	virtual ~FInworldCustomEvent() = default;

	virtual void Accept(InworldPacketVisitor & Visitor) override { Visitor.Visit(*this); }
		
	UPROPERTY()
	FString Name;

	UPROPERTY()
	FInworldReplicatedMapStruct Params;
	
protected:
	virtual void AppendDebugString(FString& Str) const override;
};

USTRUCT()
struct INWORLDAICLIENT_API FInworldRelationEvent : public FInworldPacket
{
	GENERATED_BODY()

	FInworldRelationEvent() = default;
	virtual ~FInworldRelationEvent() = default;

	virtual void Accept(InworldPacketVisitor& Visitor) override { Visitor.Visit(*this); }

	UPROPERTY()
	int32 Attraction = 0;

	UPROPERTY()
	int32 Familiar = 0;

	UPROPERTY()
	int32 Flirtatious = 0;

	UPROPERTY()
	int32 Respect = 0;

	UPROPERTY()
	int32 Trust = 0;

protected:
	virtual void AppendDebugString(FString& Str) const override;
};
