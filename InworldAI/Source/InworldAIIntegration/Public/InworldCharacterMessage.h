/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"

#include <string>

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

	virtual bool IsReady() const { return true; }

	virtual void AcceptHandle(ICharacterMessageVisitor& Visitor) PURE_VIRTUAL(FCharacterMessage::AcceptHandle)

	virtual void AcceptInterrupt(ICharacterMessageVisitor& Visitor) PURE_VIRTUAL(FCharacterMessage::AcceptInterrupt)

	virtual void AcceptCancel(ICharacterMessageVisitor& Visitor) PURE_VIRTUAL(FCharacterMessage::AcceptCancel)
};

USTRUCT(BlueprintType)
struct FCharacterUtteranceVisemeInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Message")
	FString Code;

	UPROPERTY(BlueprintReadOnly, Category = "Message")
	float Timestamp;
};

USTRUCT(BlueprintType)
struct FCharacterMessageUtterance : public FCharacterMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Message")
	FString Text;

	UPROPERTY(BlueprintReadOnly, Category = "Message")
	TArray<FCharacterUtteranceVisemeInfo> VisemeInfos;

	bool bTextFinal = false;
	bool bAudioFinal = false;
	std::string AudioData;

	virtual bool IsReady() const override { return bTextFinal && bAudioFinal; }

	virtual void AcceptHandle(ICharacterMessageVisitor& Visitor) override { Visitor.Handle(*this); }
	virtual void AcceptInterrupt(ICharacterMessageVisitor& Visitor) override { Visitor.Interrupt(*this); }
	virtual void AcceptCancel(ICharacterMessageVisitor& Visitor) override { }
};

USTRUCT(BlueprintType)
struct FCharacterMessagePlayerTalk : public FCharacterMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Message")
	FString Text;

	UPROPERTY(BlueprintReadOnly, Category = "Message")
	bool bTextFinal;

	virtual void AcceptHandle(ICharacterMessageVisitor& Visitor) override { }
	virtual void AcceptInterrupt(ICharacterMessageVisitor& Visitor) override { }
	virtual void AcceptCancel(ICharacterMessageVisitor& Visitor) override { }
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
};

USTRUCT(BlueprintType)
struct FCharacterMessageTrigger : public FCharacterMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Message")
	FString Name;

	virtual bool IsReady() const override { return !Name.IsEmpty(); }

	virtual void AcceptHandle(ICharacterMessageVisitor& Visitor) override { Visitor.Handle(*this); }
	virtual void AcceptInterrupt(ICharacterMessageVisitor& Visitor) override { }
	virtual void AcceptCancel(ICharacterMessageVisitor& Visitor) override { Visitor.Handle(*this); }
};

USTRUCT(BlueprintType)
struct FCharacterMessageInteractionEnd : public FCharacterMessage
{
	GENERATED_BODY()

	virtual void AcceptHandle(ICharacterMessageVisitor& Visitor) override { Visitor.Handle(*this); }
	virtual void AcceptInterrupt(ICharacterMessageVisitor& Visitor) override { }
	virtual void AcceptCancel(ICharacterMessageVisitor& Visitor) override { Visitor.Handle(*this); }
};

struct FCharacterMessageQueue : public TSharedFromThis<FCharacterMessageQueue>
{
	FCharacterMessageQueue()
		: FCharacterMessageQueue(nullptr)
	{}
	FCharacterMessageQueue(class ICharacterMessageVisitor* InMessageVisitor)
		: MessageVisitor(InMessageVisitor)
	{}

	class ICharacterMessageVisitor* MessageVisitor;

	TSharedPtr<FCharacterMessage> CurrentMessage;

	struct FCharacterMessageQueueEntry
	{
		FCharacterMessageQueueEntry(TSharedPtr<FCharacterMessage> InMessage, float InTimestamp)
			: Message(InMessage)
			, Timestamp(InTimestamp)
		{}
		TSharedPtr<FCharacterMessage> Message;
		float Timestamp;
	};

	TArray<FCharacterMessageQueueEntry> PendingMessageEntries;

	template<class T>
	void AddOrUpdateMessage(float Timestamp, const FString& InteractionId, const FString& UtteranceId, TFunction<void(TSharedPtr<T> MessageToPopulate)> PopulateProperties = nullptr)
	{
		TSharedPtr<T> Message = nullptr;
		if (FCharacterMessageQueueEntry* QueueEntryPtr = PendingMessageEntries.FindByPredicate([&InteractionId, &UtteranceId](const auto& Q) { return Q.Message->InteractionId == InteractionId && Q.Message->UtteranceId == UtteranceId; }))
		{
			Message = StaticCastSharedPtr<T>(QueueEntryPtr->Message);
		}

		if (!Message.IsValid())
		{
			Message = MakeShared<T>();
			Message->InteractionId = InteractionId;
			Message->UtteranceId = UtteranceId;
			PendingMessageEntries.Emplace(Message, Timestamp);
		}
		if (PopulateProperties)
		{
			PopulateProperties(Message);
		}
		TryToProgress();
	}

	TArray<FString> CancelInteraction(const FString& InteractionId);
	void TryToProgress(bool bForce = false);
	TOptional<float> GetBlockingTimestamp() const;
	void Clear();

	int32 LockCount = 0;
	TSharedPtr<struct FCharacterMessageQueueLock> MakeLock();
};

struct FCharacterMessageQueueLock
{
	FCharacterMessageQueueLock(TSharedRef<FCharacterMessageQueue> InQueue);
	~FCharacterMessageQueueLock();

	TWeakPtr<FCharacterMessageQueue> QueuePtr;
	TWeakPtr<FCharacterMessage> MessagePtr;
};

USTRUCT(BlueprintType)
struct FInworldCharacterMessageQueueLockHandle
{
	GENERATED_BODY()

	TSharedPtr<FCharacterMessageQueueLock> Lock;
};
