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

	/** The unique identifier for the utterance message. */
	UPROPERTY(BlueprintReadOnly, Category = "Message")
	FString UtteranceId;

	/** The unique identifier for the interaction message. */
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

	/** The code representing the viseme. */
	UPROPERTY(BlueprintReadOnly, Category = "Message")
	FString Code;

	/** The timestamp for the viseme. */
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
	virtual ~FCharacterMessageUtteranceData() {}
private:
	const ECharacterMessageUtteranceDataType Type;

protected:
	FCharacterMessageUtteranceData(ECharacterMessageUtteranceDataType InType)
		: Type(InType)
	{}

public:
	virtual bool IsReady() const { return true; }
	virtual bool IsFinal() const { return true; }

	template<class T>
	bool IsType() { return false; }
};

struct FCharacterMessageUtteranceDataAudio : public FCharacterMessageUtteranceData
{
public:
	FCharacterMessageUtteranceDataAudio() = default;
	virtual ~FCharacterMessageUtteranceDataAudio() = default;
protected:
	FCharacterMessageUtteranceDataAudio(ECharacterMessageUtteranceDataType InType)
		: FCharacterMessageUtteranceData(InType)
	{}
public:
	TArray<uint8> SoundData;
	int32 ChannelCount = 0;
	int32 SamplesPerSecond = 0;
	int32 BitsPerSample = 0;
	bool bAudioFinal = false;

	virtual bool IsReady() const override { return !SoundData.IsEmpty(); }
	virtual bool IsFinal() const override { return bAudioFinal; }
};

struct FCharacterMessageUtteranceDataInworld : public FCharacterMessageUtteranceDataAudio
{
public:
	FCharacterMessageUtteranceDataInworld()
		: FCharacterMessageUtteranceDataAudio(ECharacterMessageUtteranceDataType::INWORLD)
	{}
	virtual ~FCharacterMessageUtteranceDataInworld() = default;

	TArray<FCharacterUtteranceVisemeInfo> VisemeInfos;
};

struct FCharacterMessageUtteranceDataA2F : public FCharacterMessageUtteranceDataAudio
{
public:
	FCharacterMessageUtteranceDataA2F()
		: FCharacterMessageUtteranceDataAudio(ECharacterMessageUtteranceDataType::A2F)
	{}

	virtual ~FCharacterMessageUtteranceDataA2F() = default;

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

	/** The text of the utterance message. */
	UPROPERTY(BlueprintReadOnly, Category = "Message")
	FString Text;

	/** Flag indicating if the text is final. */
	UPROPERTY(BlueprintReadOnly, Category = "Message")
	bool bTextFinal = false;

	TSharedPtr<FCharacterMessageUtteranceData> UtteranceData;

	virtual FString ToDebugString() const override { return FString::Printf(TEXT("Utterance. Text: %s"), *Text); }
};

USTRUCT(BlueprintType)
struct FCharacterMessagePlayerTalk : public FCharacterMessage
{
	GENERATED_BODY()

	/** The text of the player talk message. */
	UPROPERTY(BlueprintReadOnly, Category = "Message")
	FString Text;

	/** Flag indicating if the text is final. */
	UPROPERTY(BlueprintReadOnly, Category = "Message")
	bool bTextFinal = false;

	virtual FString ToDebugString() const override { return FString::Printf(TEXT("PlayerTalk. Text: %s"), *Text); }
};

USTRUCT(BlueprintType)
struct FCharacterMessageSilence : public FCharacterMessage
{
	GENERATED_BODY()

	/** The duration of the silence message. */
	UPROPERTY(BlueprintReadOnly, Category = "Message")
	float Duration = 0.f;

	virtual FString ToDebugString() const override { return FString::Printf(TEXT("Silence. Duration: %f"), Duration); }
};

USTRUCT(BlueprintType)
struct FCharacterMessageTrigger : public FCharacterMessage
{
	GENERATED_BODY()

	/** The name of the trigger message. */
	UPROPERTY(BlueprintReadOnly, Category = "Message")
	FString Name;

	/** Parameters associated with the trigger. */
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
