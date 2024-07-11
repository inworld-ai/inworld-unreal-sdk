/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */
#include "InworldCharacterMessageQueue.h"
#include "InworldAIIntegrationModule.h"

void FCharacterMessageQueue::Interrupt()
{
	Interrupting = true;

	if (CurrentMessageQueueEntry.IsValid())
	{
		CurrentMessageQueueEntry->AcceptInterrupt(*MessageVisitor);

		// Current Message and its lock should never be valid after AcceptInterrupt, as users should clear handle on interrupt.
		// However, since this can not be guaranteed, make lock invalid if it is still lingering.
		if (CurrentMessageQueueEntry.IsValid())
		{
			auto LockPinned = QueueLock.Pin();
			if (LockPinned.IsValid())
			{
				LockPinned->Valid = false;
			}
		}
	}
	CurrentMessageQueueEntry = nullptr;

	for (const TSharedPtr<FCharacterMessageQueueEntryBase>& EntryToCancel : PendingMessageQueueEntries)
	{
		EntryToCancel->AcceptCancel(*MessageVisitor);
	}

	PendingMessageQueueEntries.Empty();

	Interrupting = false;
}

void FCharacterMessageQueue::TryToProgress()
{
	while (!CurrentMessageQueueEntry.IsValid() && !Interrupting)
	{
		if (PendingMessageQueueEntries.Num() == 0)
		{
			return;
		}

		auto NextQueuedEntry = PendingMessageQueueEntries[0];
		if(!NextQueuedEntry->IsReady())
		{
			return;
		}

		CurrentMessageQueueEntry = NextQueuedEntry;
		PendingMessageQueueEntries.RemoveAt(0);

		auto CurrentMessage = CurrentMessageQueueEntry->GetCharacterMessage();
		UE_LOG(LogInworldAIIntegration, Log, TEXT("Handle character message '%s::%s'"), *CurrentMessage->InteractionId, *CurrentMessage->UtteranceId);

		TSharedPtr<FCharacterMessageQueueLock> LockPinned = MakeLock();
		QueueLock = LockPinned;
		CurrentMessageQueueEntry->AcceptHandle(*MessageVisitor);
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
		QueuePinned->CurrentMessageQueueEntry = nullptr;
		QueuePinned->TryToProgress();
	}
}

bool FCharacterMessageQueue::Lock(FInworldCharacterMessageQueueLockHandle& LockHandle)
{
	auto LockPinned = QueueLock.Pin();
	if (!LockPinned.IsValid())
	{
		return false;
	}

	auto QueuePinned = LockPinned->QueuePtr.Pin();
	if (!QueuePinned.IsValid())
	{
		return false;
	}

	if (!CurrentMessageQueueEntry.IsValid())
	{
		return false;
	}

	LockHandle.Lock = LockPinned;
	return true;
}

void FCharacterMessageQueue::Unlock(FInworldCharacterMessageQueueLockHandle& LockHandle)
{
	LockHandle.Lock = {};
}
