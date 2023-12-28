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
struct FCharacterMessageTrigger;
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

struct FCharacterMessageQueue : public TSharedFromThis<FCharacterMessageQueue>
{
	FCharacterMessageQueue()
		: FCharacterMessageQueue(nullptr)
	{}
	FCharacterMessageQueue(class ICharacterMessageVisitor* InMessageVisitor)
		: MessageVisitor(InMessageVisitor)
	{
	}

	class ICharacterMessageVisitor* MessageVisitor;

	TSharedPtr<FCharacterMessage> CurrentMessage;
	TArray<TSharedPtr<FCharacterMessage>> PendingMessageEntries;

	template<class T>
	void AddOrUpdateMessage(const FInworldPacket& Event, TFunction<void(TSharedPtr<T> MessageToPopulate)> PopulateProperties = nullptr)
	{
		const FString& InteractionId = Event.PacketId.InteractionId;
		const FString& UtteranceId = Event.PacketId.UtteranceId;

		TSharedPtr<T> Message = nullptr;
		const auto Index = PendingMessageEntries.FindLastByPredicate([&InteractionId, &UtteranceId](const TSharedPtr<FCharacterMessage>& MessageQueueEntry)
			{
				return MessageQueueEntry->InteractionId == InteractionId && MessageQueueEntry->UtteranceId == UtteranceId;
			}
		);
		if (Index != INDEX_NONE)
		{
			Message = StaticCastSharedPtr<T>(PendingMessageEntries[Index]);
		}

		if (!Message.IsValid() || Message->IsReady())
		{
			Message = MakeShared<T>();
			Message->InteractionId = InteractionId;
			Message->UtteranceId = UtteranceId;
			PendingMessageEntries.Emplace(Message);
		}
		if (PopulateProperties)
		{
			PopulateProperties(Message);
		}

		TryToProgress();
	}

	TMap<FString, TArray<FString>> Interrupt();
	void TryToProgress();

private:
	bool Locked = false;
	bool Interrupting = false;
	TSharedPtr<struct FCharacterMessageQueueLock> MakeLock();
	friend struct FCharacterMessageQueueLock;
};

struct FCharacterMessageQueueLock
{
	FCharacterMessageQueueLock(TSharedRef<FCharacterMessageQueue> InQueue);
	~FCharacterMessageQueueLock();

	UPROPERTY()
	TWeakPtr<FCharacterMessageQueue> QueuePtr;

	UPROPERTY()
	bool Valid = true;
};

USTRUCT(BlueprintType)
struct FInworldCharacterMessageQueueLockHandle
{
	GENERATED_BODY()

	TSharedPtr<FCharacterMessageQueueLock> Lock;
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

	TWeakPtr<FCharacterMessageQueueLock> Lock;

	virtual bool IsReady() const { return true; }

	virtual void AcceptHandle(ICharacterMessageVisitor& Visitor) PURE_VIRTUAL(FCharacterMessage::AcceptHandle)
	virtual void AcceptInterrupt(ICharacterMessageVisitor& Visitor) PURE_VIRTUAL(FCharacterMessage::AcceptInterrupt)
	virtual void AcceptCancel(ICharacterMessageVisitor& Visitor) PURE_VIRTUAL(FCharacterMessage::AcceptCancel)

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

	virtual bool IsReady() const override { return bTextFinal && bAudioFinal; }

	virtual void AcceptHandle(ICharacterMessageVisitor& Visitor) override { Visitor.Handle(*this); }
	virtual void AcceptInterrupt(ICharacterMessageVisitor& Visitor) override { Visitor.Interrupt(*this); }
	virtual void AcceptCancel(ICharacterMessageVisitor& Visitor) override { }

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

	virtual void AcceptHandle(ICharacterMessageVisitor& Visitor) override { }
	virtual void AcceptInterrupt(ICharacterMessageVisitor& Visitor) override { }
	virtual void AcceptCancel(ICharacterMessageVisitor& Visitor) override { }

	virtual FString ToDebugString() const override { return FString::Printf(TEXT("PlayerTalk. Text: %s"), *Text); }
};

USTRUCT(BlueprintType)
struct FCharacterMessageSilence : public FCharacterMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Message")
	float Duration = 0.f;

	virtual bool IsReady() const override { return Duration != 0.f; }

	virtual void AcceptHandle(ICharacterMessageVisitor& Visitor) override { Visitor.Handle(*this); }
	virtual void AcceptInterrupt(ICharacterMessageVisitor& Visitor) override { Visitor.Interrupt(*this); }
	virtual void AcceptCancel(ICharacterMessageVisitor& Visitor) override { }

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

	virtual bool IsReady() const override { return !Name.IsEmpty(); }

	virtual void AcceptHandle(ICharacterMessageVisitor& Visitor) override { Visitor.Handle(*this); }
	virtual void AcceptInterrupt(ICharacterMessageVisitor& Visitor) override { }
	virtual void AcceptCancel(ICharacterMessageVisitor& Visitor) override { Visitor.Handle(*this); }

	virtual FString ToDebugString() const override { return FString::Printf(TEXT("Trigger. Name: %s"), *Name); }
};

USTRUCT(BlueprintType)
struct FCharacterMessageInteractionEnd : public FCharacterMessage
{
	GENERATED_BODY()

	virtual void AcceptHandle(ICharacterMessageVisitor& Visitor) override { Visitor.Handle(*this); }
	virtual void AcceptInterrupt(ICharacterMessageVisitor& Visitor) override { }
	virtual void AcceptCancel(ICharacterMessageVisitor& Visitor) override { Visitor.Handle(*this); }

	virtual FString ToDebugString() const override { return TEXT("InteractionEnd"); }
};

UCLASS()
class INWORLDAIINTEGRATION_API UInworldCharacterMessageQueueFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Inworld|Character")
	static bool LockMessage(const FCharacterMessage& Message, UPARAM(ref) FInworldCharacterMessageQueueLockHandle& LockHandle);

	UFUNCTION(BlueprintCallable, Category = "Inworld|Character")
	static void UnlockMessage(UPARAM(ref) FInworldCharacterMessageQueueLockHandle& LockHandle);
};
