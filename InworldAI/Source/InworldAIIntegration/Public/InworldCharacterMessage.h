/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "Containers/Union.h"

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
	virtual void Pause(const FCharacterMessageUtterance& Event) { }
	virtual void Resume(const FCharacterMessageUtterance& Event) { }

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

	virtual FString ToDebugString() const PURE_VIRTUAL(FCharacterMessage::ToDebugString, return FString();)
};

USTRUCT(BlueprintType)
struct FCharacterUtteranceVisemeInfo
{
	GENERATED_BODY()
	FCharacterUtteranceVisemeInfo() = default;
	FCharacterUtteranceVisemeInfo(const FString& InCode, float InTimestamp)
		: Code(InCode)
		, Timestamp(InTimestamp)
	{}

	UPROPERTY(BlueprintReadOnly, Category = "Message")
	FString Code;

	UPROPERTY(BlueprintReadOnly, Category = "Message")
	float Timestamp = 0.f;
};

enum class ECharacterMessageUtteranceDataType
{
	UNKNOWN,
	INWORLD,
	A2F,
};

struct FCharacterMessageUtteranceData : public TSharedFromThis<FCharacterMessageUtteranceData>
{
public:
	FCharacterMessageUtteranceData()
		: FCharacterMessageUtteranceData(ECharacterMessageUtteranceDataType::UNKNOWN)
	{}

private:
	const ECharacterMessageUtteranceDataType Type;

protected:
	FCharacterMessageUtteranceData(ECharacterMessageUtteranceDataType InType)
		: Type(InType)
	{}

public:
	TArray<uint8> SoundData;
	int32 ChannelCount = 0;
	int32 SamplesPerSecond = 0;
	int32 BitsPerSample = 0;
	bool bAudioFinal = false;

	template<class T>
	bool IsType() { return false; }
};

struct FCharacterMessageUtteranceDataInworld : public FCharacterMessageUtteranceData
{
	FCharacterMessageUtteranceDataInworld()
		: FCharacterMessageUtteranceData(ECharacterMessageUtteranceDataType::INWORLD)
	{}

	TArray<FCharacterUtteranceVisemeInfo> VisemeInfos;
};

struct FCharacterMessageUtteranceDataA2F : public FCharacterMessageUtteranceData
{
	FCharacterMessageUtteranceDataA2F()
		: FCharacterMessageUtteranceData(ECharacterMessageUtteranceDataType::A2F)
	{}

	bool bRecvEnd = false;
	TArray<FName> BlendShapeNames;
	TArray<TMap<FName, float>> BlendShapeMaps;
};

template<>
bool FCharacterMessageUtteranceData::IsType<FCharacterMessageUtteranceDataInworld>();
template<>
bool FCharacterMessageUtteranceData::IsType<FCharacterMessageUtteranceDataA2F>();

USTRUCT(BlueprintType)
struct FCharacterMessageUtterance : public FCharacterMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Message")
	FString Text;

	UPROPERTY(BlueprintReadOnly, Category = "Message")
	bool bTextFinal = false;

	TSharedPtr<FCharacterMessageUtteranceData> UtteranceData;

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

	virtual FString ToDebugString() const override { return FString::Printf(TEXT("PlayerTalk. Text: %s"), *Text); }
};

USTRUCT(BlueprintType)
struct FCharacterMessageSilence : public FCharacterMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Message")
	float Duration = 0.f;

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

	virtual FString ToDebugString() const override { return FString::Printf(TEXT("Trigger. Name: %s"), *Name); }
};

USTRUCT(BlueprintType)
struct FCharacterMessageInteractionEnd : public FCharacterMessage
{
	GENERATED_BODY()

	virtual FString ToDebugString() const override { return TEXT("InteractionEnd"); }
};

void operator<<(FCharacterMessage& Message, const FInworldPacket& Packet);
void operator<<(FCharacterMessageUtterance& Message, const FInworldTextEvent& Event);
void operator<<(FCharacterMessageUtterance& Message, const FInworldAudioDataEvent& Event);
void operator<<(FCharacterMessageUtterance& Message, const FInworldA2FHeaderEvent& Event);
void operator<<(FCharacterMessageUtterance& Message, const FInworldA2FContentEvent& Event);
void operator<<(FCharacterMessagePlayerTalk& Message, const FInworldTextEvent& Event);
void operator<<(FCharacterMessageSilence& Message, const FInworldSilenceEvent& Event);
void operator<<(FCharacterMessageTrigger& Message, const FInworldCustomEvent& Event);
void operator<<(FCharacterMessageTrigger& Message, const FInworldRelationEvent& Event);
void operator<<(FCharacterMessageInteractionEnd& Message, const FInworldControlEvent& Event);
