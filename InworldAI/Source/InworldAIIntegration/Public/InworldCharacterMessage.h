/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"

#include "InworldPackets.h"

#include "InworldCharacterMessage.generated.h"

struct FCharacterMessageUtterance;
struct FCharacterMessageSilence;
struct FCharacterMessageInteractionEnd;

class ICharacterMessageVisitor
{
public:
	virtual void Handle(const FCharacterMessageUtterance& Event) { }
	virtual void Interrupt(const FCharacterMessageUtterance& Event) { }

	virtual void Handle(const FCharacterMessageSilence& Event) { }
	virtual void Interrupt(const FCharacterMessageSilence& Event) { }

	virtual void Handle(const FCharacterMessageTrigger& Event) { }

	virtual void Handle(const FCharacterMessageInteractionEnd& Event) { }
};

USTRUCT(BlueprintType)
struct FCharacterMessage
{
	GENERATED_BODY()

	FCharacterMessage() {}
	virtual ~FCharacterMessage() = default;

	UPROPERTY(BlueprintReadOnly, Category = "Message")
	FString UtteranceId;

	UPROPERTY(BlueprintReadOnly, Category = "Message")
	FString InteractionId;

	void Populate(const FInworldPacket& Packet)
	{
		UtteranceId = Packet.PacketId.UtteranceId;
		InteractionId = Packet.PacketId.InteractionId;
	}

	virtual FString ToDebugString() const PURE_VIRTUAL(FCharacterMessage::ToDebugString, return FString();)
};

USTRUCT(BlueprintType)
struct FCharacterUtteranceVisemeInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Message")
	FString Code;

	UPROPERTY(BlueprintReadOnly, Category = "Message")
	float Timestamp = 0.f;
};

USTRUCT(BlueprintType)
struct FCharacterMessageUtterance : public FCharacterMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Message")
	FString Text;

	UPROPERTY(BlueprintReadOnly, Category = "Message")
	TArray<FCharacterUtteranceVisemeInfo> VisemeInfos;

	UPROPERTY(BlueprintReadOnly, Category = "Message")
	bool bTextFinal = false;

	UPROPERTY(BlueprintReadOnly, Category = "Message")
	bool bAudioFinal = false;

	UPROPERTY(BlueprintReadOnly, Category = "Message")
	TArray<uint8> SoundData;

	void Populate(const FInworldTextEvent& Event)
	{
		FCharacterMessage::Populate(Event);
		Text = Event.Text;
		bTextFinal = Event.Final;
	}

	void Populate(const FInworldAudioDataEvent& Event)
	{
		FCharacterMessage::Populate(Event);
		SoundData.Append(Event.Chunk);

		ensure(!bAudioFinal);
		bAudioFinal = Event.bFinal;

		auto& InworldVisemeInfos = Event.VisemeInfos;
		VisemeInfos.Reserve(InworldVisemeInfos.Num());
		for (auto& VisemeInfo : InworldVisemeInfos)
		{
			FCharacterUtteranceVisemeInfo& VisemeInfo_Ref = VisemeInfos.AddDefaulted_GetRef();
			VisemeInfo_Ref.Timestamp = VisemeInfo.Timestamp;
			VisemeInfo_Ref.Code = VisemeInfo.Code;
		}
	}

	virtual FString ToDebugString() const override { return FString::Printf(TEXT("Utterance. Text: %s"), *Text); }
};

USTRUCT(BlueprintType)
struct FCharacterMessagePlayerTalk : public FCharacterMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Message")
	FString Text;

	UPROPERTY(BlueprintReadOnly, Category = "Message")
	bool bTextFinal = false;

	void Populate(const FInworldTextEvent& Event)
	{
		FCharacterMessage::Populate(Event);
		Text = Event.Text;
		bTextFinal = Event.Final;
	}

	virtual FString ToDebugString() const override { return FString::Printf(TEXT("PlayerTalk. Text: %s"), *Text); }
};

USTRUCT(BlueprintType)
struct FCharacterMessageSilence : public FCharacterMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Message")
	float Duration = 0.f;

	void Populate(const FInworldSilenceEvent& Event)
	{
		FCharacterMessage::Populate(Event);
		Duration = Event.Duration;
	}

	virtual FString ToDebugString() const override { return FString::Printf(TEXT("Silence. Duration: %f"), Duration); }
};

USTRUCT(BlueprintType)
struct FCharacterMessageTrigger : public FCharacterMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Message")
	FString Name;

	UPROPERTY(BlueprintReadOnly, Category = "Message")
	TMap<FString, FString> Params;

	void Populate(const FInworldCustomEvent& Event)
	{
		FCharacterMessage::Populate(Event);
		Name = Event.Name;
		Params = Event.Params.RepMap;
	}

	void Populate(const FInworldRelationEvent& Event)
	{
		FCharacterMessage::Populate(Event);
		Name = TEXT("inworld.relation.update");
		Params.Add(TEXT("Attraction"), FString::FromInt(Event.Attraction));
		Params.Add(TEXT("Familiar"), FString::FromInt(Event.Familiar));
		Params.Add(TEXT("Flirtatious"), FString::FromInt(Event.Flirtatious));
		Params.Add(TEXT("Respect"), FString::FromInt(Event.Respect));
		Params.Add(TEXT("Trust"), FString::FromInt(Event.Trust));
	}

	virtual FString ToDebugString() const override { return FString::Printf(TEXT("Trigger. Name: %s"), *Name); }
};

USTRUCT(BlueprintType)
struct FCharacterMessageInteractionEnd : public FCharacterMessage
{
	GENERATED_BODY()

	void Populate(const FInworldControlEvent& event) { }

	virtual FString ToDebugString() const override { return TEXT("InteractionEnd"); }
};
