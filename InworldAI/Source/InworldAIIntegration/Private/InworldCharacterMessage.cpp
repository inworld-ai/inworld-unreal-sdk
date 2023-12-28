/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */
#include "InworldCharacterMessage.h"
#include "InworldAIIntegrationModule.h"

TMap<FString, TArray<FString>> FCharacterMessageQueue::Interrupt()
{
	Interrupting = true;

	TMap<FString, TArray<FString>> InterruptedInteractions;

	if (CurrentMessage.IsValid())
	{
		InterruptedInteractions.Add(CurrentMessage->InteractionId, { CurrentMessage->UtteranceId });
		CurrentMessage->AcceptInterrupt(*MessageVisitor);

		// Current Message and its lock should never be valid after AcceptInterrupt, as users should clear handle on interrupt.
		// However, since this can not be guaranteed, make lock invalid if it is still lingering.
		if (CurrentMessage.IsValid())
		{
			auto LockPinned = CurrentMessage->Lock.Pin();
			if (LockPinned.IsValid())
			{
				LockPinned->Valid = false;
			}
		}
	}
	CurrentMessage = nullptr;

	for (const TSharedPtr<FCharacterMessage>& EntryToCancel : PendingMessageEntries)
	{
		const FString& InteractionId = EntryToCancel->InteractionId;
		const FString& UtteranceId = EntryToCancel->UtteranceId;
		if (!InterruptedInteractions.Contains(InteractionId))
		{
			InterruptedInteractions.Add(InteractionId, {});
		}
		InterruptedInteractions[InteractionId].Add(UtteranceId);
		EntryToCancel->AcceptCancel(*MessageVisitor);
	}

	PendingMessageEntries.Empty();

	Interrupting = false;

	return InterruptedInteractions;
}

void FCharacterMessageQueue::TryToProgress()
{
	while (!CurrentMessage.IsValid() && !Interrupting)
	{
		if (PendingMessageEntries.Num() == 0)
		{
			return;
		}

		auto NextQueuedEntry = PendingMessageEntries[0];
		if(!NextQueuedEntry->IsReady())
		{
			return;
		}

		CurrentMessage = NextQueuedEntry;
		PendingMessageEntries.RemoveAt(0);

		UE_LOG(LogInworldAIIntegration, Log, TEXT("Handle character message '%s::%s'"), *CurrentMessage->InteractionId, *CurrentMessage->UtteranceId);
		auto LockPinned = MakeLock();
		CurrentMessage->Lock = LockPinned;
		CurrentMessage->AcceptHandle(*MessageVisitor);
	}
}

TSharedPtr<FCharacterMessageQueueLock> FCharacterMessageQueue::MakeLock()
{
	return MakeShared<FCharacterMessageQueueLock>(AsShared());
}

FCharacterMessageQueueLock::FCharacterMessageQueueLock(TSharedRef<FCharacterMessageQueue> InQueue)
	: QueuePtr(InQueue)
{}

FCharacterMessageQueueLock::~FCharacterMessageQueueLock()
{
	if (!Valid)
	{
		return;
	}

	auto QueuePinned = QueuePtr.Pin();
	if (QueuePinned.IsValid())
	{
		QueuePinned->CurrentMessage = nullptr;
		QueuePinned->TryToProgress();
	}
}

bool UInworldCharacterMessageQueueFunctionLibrary::LockMessage(const FCharacterMessage& Message, FInworldCharacterMessageQueueLockHandle& LockHandle)
{
	auto LockPinned = Message.Lock.Pin();
	if (!LockPinned.IsValid())
	{
		return false;
	}

	auto QueuePinned = LockPinned->QueuePtr.Pin();
	if (!QueuePinned.IsValid())
	{
		return false;
	}

	auto CurrentMessage = QueuePinned->CurrentMessage;
	if (!CurrentMessage.IsValid())
	{
		return false;
	}

	if (CurrentMessage->InteractionId != Message.InteractionId || CurrentMessage->UtteranceId != Message.UtteranceId)
	{
		return false;
	}

	LockHandle.Lock = LockPinned;
	return true;
}

void UInworldCharacterMessageQueueFunctionLibrary::UnlockMessage(FInworldCharacterMessageQueueLockHandle& LockHandle)
{
	LockHandle.Lock = {};
}
