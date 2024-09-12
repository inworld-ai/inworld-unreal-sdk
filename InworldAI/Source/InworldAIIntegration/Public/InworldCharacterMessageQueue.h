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
	virtual void AcceptPause(ICharacterMessageVisitor& Visitor) = 0;
	virtual void AcceptResume(ICharacterMessageVisitor& Visitor) = 0;
	virtual void AcceptCancel(ICharacterMessageVisitor& Visitor) = 0;

	virtual bool IsReady() const = 0;
	virtual bool IsFinished() const = 0;
	virtual bool IsEnd() const = 0;
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
	virtual void AcceptPause(ICharacterMessageVisitor& Visitor) override;
	virtual void AcceptResume(ICharacterMessageVisitor& Visitor) override;
	virtual void AcceptCancel(ICharacterMessageVisitor& Visitor) override;
	virtual bool IsReady() const override;
	virtual bool IsFinished() const override;
	virtual bool IsEnd() const override;
};

template<class T>
void FCharacterMessageQueueEntry<T>::AcceptHandle(ICharacterMessageVisitor& Visitor) { Visitor.Handle(*Message); }
template<class T>
void FCharacterMessageQueueEntry<T>::AcceptInterrupt(ICharacterMessageVisitor& Visitor) { Visitor.Interrupt(*Message); }
template<class T>
void FCharacterMessageQueueEntry<T>::AcceptPause(ICharacterMessageVisitor& Visitor) { Visitor.Pause(*Message); }
template<class T>
void FCharacterMessageQueueEntry<T>::AcceptResume(ICharacterMessageVisitor& Visitor) { Visitor.Resume(*Message); }
template<class T>
void FCharacterMessageQueueEntry<T>::AcceptCancel(ICharacterMessageVisitor& Visitor) { }
template<class T>
bool FCharacterMessageQueueEntry<T>::IsReady() const { return true; }
template<class T>
bool FCharacterMessageQueueEntry<T>::IsFinished() const { return true; }
template<class T>
bool FCharacterMessageQueueEntry<T>::IsEnd() const { return false; }

template<>
inline bool FCharacterMessageQueueEntry<FCharacterMessageUtterance>::IsReady() const
{
	return Message->bTextFinal && Message->UtteranceData && !Message->UtteranceData->SoundData.IsEmpty();
}
template<>
inline bool FCharacterMessageQueueEntry<FCharacterMessageUtterance>::IsFinished() const
{
	return Message->bTextFinal && Message->UtteranceData && Message->UtteranceData->bAudioFinal;
}

template<>
inline bool FCharacterMessageQueueEntry<FCharacterMessageSilence>::IsReady() const { return Message->Duration != 0; }
template<>
inline void FCharacterMessageQueueEntry<FCharacterMessageSilence>::AcceptPause(ICharacterMessageVisitor& Visitor) { }
template<>
inline void FCharacterMessageQueueEntry<FCharacterMessageSilence>::AcceptResume(ICharacterMessageVisitor& Visitor) { }

template<>
inline void FCharacterMessageQueueEntry<FCharacterMessageTrigger>::AcceptInterrupt(ICharacterMessageVisitor& Visitor) { Visitor.Handle(*Message); }
template<>
inline void FCharacterMessageQueueEntry<FCharacterMessageTrigger>::AcceptCancel(ICharacterMessageVisitor& Visitor) { Visitor.Handle(*Message); }
template<>
inline void FCharacterMessageQueueEntry<FCharacterMessageTrigger>::AcceptPause(ICharacterMessageVisitor& Visitor) { }
template<>
inline void FCharacterMessageQueueEntry<FCharacterMessageTrigger>::AcceptResume(ICharacterMessageVisitor& Visitor) { }

template<>
inline void FCharacterMessageQueueEntry<FCharacterMessageInteractionEnd>::AcceptInterrupt(ICharacterMessageVisitor& Visitor) { Visitor.Handle(*Message); }
template<>
inline void FCharacterMessageQueueEntry<FCharacterMessageInteractionEnd>::AcceptCancel(ICharacterMessageVisitor& Visitor) { Visitor.Handle(*Message); }
template<>
inline void FCharacterMessageQueueEntry<FCharacterMessageInteractionEnd>::AcceptPause(ICharacterMessageVisitor& Visitor) { }
template<>
inline void FCharacterMessageQueueEntry<FCharacterMessageInteractionEnd>::AcceptResume(ICharacterMessageVisitor& Visitor) { }
template<>
inline bool FCharacterMessageQueueEntry<FCharacterMessageInteractionEnd>::IsEnd() const { return true; }

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

	template<class T, class U>
	void AddOrUpdateMessage(const T& Event)
	{
		const FString& InteractionId = Event.PacketId.InteractionId;
		const FString& UtteranceId = Event.PacketId.UtteranceId;

		TSharedPtr<FCharacterMessageQueueEntryBase> MessageQueueEntry = nullptr;
		TSharedPtr<U> Message = nullptr;
		if (CurrentMessageQueueEntry.IsValid() && CurrentMessageQueueEntry->GetCharacterMessage()->InteractionId == InteractionId && CurrentMessageQueueEntry->GetCharacterMessage()->UtteranceId == UtteranceId)
		{
			MessageQueueEntry = CurrentMessageQueueEntry;
			Message = StaticCastSharedPtr<U>(CurrentMessageQueueEntry->GetCharacterMessage());
		}
		else
		{
			const auto Index = PendingMessageQueueEntries.FindLastByPredicate([&InteractionId, &UtteranceId](const TSharedPtr<FCharacterMessageQueueEntryBase> MessageQueueEntry)
				{
					return MessageQueueEntry->GetCharacterMessage()->InteractionId == InteractionId && MessageQueueEntry->GetCharacterMessage()->UtteranceId == UtteranceId;
				}
			);
			if (Index != INDEX_NONE)
			{
				MessageQueueEntry = PendingMessageQueueEntries[Index];
				Message = StaticCastSharedPtr<U>(MessageQueueEntry->GetCharacterMessage());
			}
		}

		if (CanCreateNewQueueEntry<T>() && (!MessageQueueEntry.IsValid() || MessageQueueEntry->IsFinished()))
		{
			MessageQueueEntry = MakeShared<FCharacterMessageQueueEntry<U>>(MakeShared<U>());
			Message = StaticCastSharedPtr<U>(MessageQueueEntry->GetCharacterMessage());
			PendingMessageQueueEntries.Emplace(MessageQueueEntry);
		}

		if (Message.IsValid())
		{
			(*Message) << Event;

			OnUpdated(*Message);
		}

		TryToProgress();
	}

	void TryToPause();
	void TryToResume();
	void TryToInterrupt(const FString& InterruptingInteractionId);
	void TryToProgress();

	bool Lock(FInworldCharacterMessageQueueLockHandle& LockHandle);
	void Unlock(FInworldCharacterMessageQueueLockHandle& LockHandle);

private:
	bool bIsPendingInterruptState = false;
	bool bIsInterrupting = false;
	bool bIsProgressing = false;
	bool bIsPaused = false;

	TOptional<FString> NextInterruptingInteractionId;
	TMap<FString, bool> InteractionInterruptibleState;
	enum class EInworldInteractionInterruptibleState : uint8
	{
		INTERRUPTIBLE = 0,
		UNINTERRUPTIBLE = 1,
		UNDETERMINED = 2,
		INVALID = 3,
	};

	template<class T>
	bool CanCreateNewQueueEntry() const { return true; }
	template<>
	bool CanCreateNewQueueEntry<FInworldA2FContentEvent>() const { return false; }
	template<>
	bool CanCreateNewQueueEntry<FInworldA2FHeaderEvent>() const { return false; }

	void SetInterruptible(const FString& InteractionId, bool bInterruptible);
	bool CanPauseCurrentMessageQueueEntry() const;
	void CancelInterruptiblePendingQueueEntries();
	EInworldInteractionInterruptibleState GetInteractionInterruptibleState(const FString& InteractionId) const;
	EInworldInteractionInterruptibleState GetQueueEntryInterruptibleState(const TSharedPtr<FCharacterMessageQueueEntryBase>& QueueEntry) const;

	void OnUpdated(const FCharacterMessageUtterance& Message) {}
	void OnUpdated(const FCharacterMessageSilence& Message) {}
	void OnUpdated(const FCharacterMessageTrigger& Message);
	void OnUpdated(const FCharacterMessageInteractionEnd& Message);

	void EndInteraction(const FString& InteractionId);

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