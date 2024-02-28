/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"

#include "InworldCharacterMessage.h"

#include "InworldCharacterMessageQueue.generated.h"

struct FCharacterMessageQueueLock;

struct FCharacterMessageQueueEntryBase
{
	FCharacterMessageQueueEntryBase() {};
	virtual ~FCharacterMessageQueueEntryBase() {};

	virtual TSharedPtr<FCharacterMessage> GetCharacterMessage() const = 0;

	virtual void AcceptHandle(ICharacterMessageVisitor& Visitor) = 0;
	virtual void AcceptInterrupt(ICharacterMessageVisitor& Visitor) = 0;
	virtual void AcceptCancel(ICharacterMessageVisitor& Visitor) = 0;

	virtual bool IsReady() const = 0;
};

template<class T>
struct FCharacterMessageQueueEntry : FCharacterMessageQueueEntryBase
{
	TSharedPtr<T> Message;

	FCharacterMessageQueueEntry(TSharedPtr<T> InMessage)
	{
		Message = InMessage;
	}

	virtual TSharedPtr<FCharacterMessage> GetCharacterMessage() const { return Message; }

	virtual void AcceptHandle(ICharacterMessageVisitor& Visitor) override;
	virtual void AcceptInterrupt(ICharacterMessageVisitor& Visitor) override;
	virtual void AcceptCancel(ICharacterMessageVisitor& Visitor) override;
	virtual bool IsReady() const override;
};

template<class T>
void FCharacterMessageQueueEntry<T>::AcceptHandle(ICharacterMessageVisitor& Visitor) { Visitor.Handle(*Message); }
template<class T>
void FCharacterMessageQueueEntry<T>::AcceptInterrupt(ICharacterMessageVisitor& Visitor) { Visitor.Interrupt(*Message); }
template<class T>
void FCharacterMessageQueueEntry<T>::AcceptCancel(ICharacterMessageVisitor& Visitor) { Visitor.Handle(*Message); }
template<class T>
bool FCharacterMessageQueueEntry<T>::IsReady() const { return true; }

template<>
inline void FCharacterMessageQueueEntry<FCharacterMessageUtterance>::AcceptCancel(ICharacterMessageVisitor& Visitor) { }
template<>
inline bool FCharacterMessageQueueEntry<FCharacterMessageUtterance>::IsReady() const { return Message->bTextFinal && Message->bAudioFinal; }

template<>
inline void FCharacterMessageQueueEntry<FCharacterMessageSilence>::AcceptCancel(ICharacterMessageVisitor& Visitor) { }
template<>
inline bool FCharacterMessageQueueEntry<FCharacterMessageSilence>::IsReady() const { return Message->Duration != 0; }

template<>
inline void FCharacterMessageQueueEntry<FCharacterMessageInteractionEnd>::AcceptInterrupt(ICharacterMessageVisitor& Visitor) { }

struct FCharacterMessageQueue : public TSharedFromThis<FCharacterMessageQueue>
{
	FCharacterMessageQueue()
		: FCharacterMessageQueue(nullptr)
	{}
	FCharacterMessageQueue(class ICharacterMessageVisitor* InMessageVisitor)
		: MessageVisitor(InMessageVisitor)
	{}

	class ICharacterMessageVisitor* MessageVisitor;

	TSharedPtr<FCharacterMessageQueueEntryBase> CurrentMessageQueueEntry;
	TArray<TSharedPtr<FCharacterMessageQueueEntryBase>> PendingMessageQueueEntries;

	template<class T>
	void AddOrUpdateMessage(const FInworldPacket& Event, TFunction<void(TSharedPtr<T> MessageToPopulate)> PopulateProperties = nullptr)
	{
		const FString& InteractionId = Event.PacketId.InteractionId;
		const FString& UtteranceId = Event.PacketId.UtteranceId;

		TSharedPtr<FCharacterMessageQueueEntryBase> MessageQueueEntry = nullptr;
		TSharedPtr<T> Message = nullptr;
		const auto Index = PendingMessageQueueEntries.FindLastByPredicate([&InteractionId, &UtteranceId](const TSharedPtr<FCharacterMessageQueueEntryBase> MessageQueueEntry)
			{
				return MessageQueueEntry->GetCharacterMessage()->InteractionId == InteractionId && MessageQueueEntry->GetCharacterMessage()->UtteranceId == UtteranceId;
			}
		);
		if (Index != INDEX_NONE)
		{
			MessageQueueEntry = PendingMessageQueueEntries[Index];
			Message = StaticCastSharedPtr<T>(MessageQueueEntry->GetCharacterMessage());
		}

		if (!MessageQueueEntry.IsValid() || MessageQueueEntry->IsReady())
		{
			Message = MakeShared<T>();
			Message->InteractionId = InteractionId;
			Message->UtteranceId = UtteranceId;
			PendingMessageQueueEntries.Emplace(MakeShared<FCharacterMessageQueueEntry<T>>(Message));
		}
		if (PopulateProperties)
		{
			PopulateProperties(Message);
		}

		TryToProgress();
	}

	TMap<FString, TArray<FString>> Interrupt();
	void TryToProgress();

	bool Lock(FInworldCharacterMessageQueueLockHandle& LockHandle);
	void Unlock(FInworldCharacterMessageQueueLockHandle& LockHandle);

private:
	bool Interrupting = false;
	TWeakPtr<FCharacterMessageQueueLock> QueueLock;
	TSharedPtr<FCharacterMessageQueueLock> MakeLock();
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