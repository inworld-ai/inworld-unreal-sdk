/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */
#include "InworldCharacterMessageQueue.h"

void FCharacterMessageQueue::TryToInterrupt(const FString& InterruptingInteractionId)
{
	bIsInterrupting = true;

	NextUninterruptedInteractionId = InterruptingInteractionId;

	auto ShouldPauseCurrentQueueEntry = [&]() -> bool
		{
			if (!CurrentMessageQueueEntry.IsValid()) return false;
			const FString& CurrentInteractionId = CurrentMessageQueueEntry->GetCharacterMessage()->InteractionId;
			if (CurrentInteractionId == NextUninterruptedInteractionId) return false;
			return !InteractionInterruptibleState.Contains(CurrentInteractionId);
		};

	if (ShouldPauseCurrentQueueEntry())
	{
		CurrentMessageQueueEntry->AcceptPause(*MessageVisitor);
		bIsPaused = true;
	}
	else
	{
		auto ShouldCancelCurrentQueueEntry = [&]() -> bool
			{
				if (!CurrentMessageQueueEntry.IsValid()) return false;
				const FString& CurrentInteractionId = CurrentMessageQueueEntry->GetCharacterMessage()->InteractionId;
				if (CurrentInteractionId == NextUninterruptedInteractionId) return false;
				return InteractionInterruptibleState.Contains(CurrentInteractionId) && InteractionInterruptibleState[CurrentInteractionId];
			};
		if (ShouldCancelCurrentQueueEntry())
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
			CurrentMessageQueueEntry = nullptr;
		}

		if (!CurrentMessageQueueEntry.IsValid())
		{
			auto ShouldCancelNextPendingQueueEntry = [&]() -> bool
				{
					if (PendingMessageQueueEntries.Num() == 0) return false;
					const FString& NextInteractionId = PendingMessageQueueEntries[0]->GetCharacterMessage()->InteractionId;
					if (NextInteractionId == NextUninterruptedInteractionId) return false;
					return InteractionInterruptibleState.Contains(NextInteractionId) && InteractionInterruptibleState[NextInteractionId];
				};
			while (ShouldCancelNextPendingQueueEntry())
			{
				PendingMessageQueueEntries[0]->AcceptCancel(*MessageVisitor);
				PendingMessageQueueEntries.RemoveAt(0);
			}

			if (!bIsProgressing)
			{
				TryToProgress();
			}
		}
	}

	bIsInterrupting = false;
}

void FCharacterMessageQueue::SetInterruptible(const FString& InteractionId, bool bInterruptible)
{
	InteractionInterruptibleState.Add(InteractionId, bInterruptible);
	if (bIsPaused && CurrentMessageQueueEntry->GetCharacterMessage()->InteractionId == InteractionId)
	{
		if (bInterruptible)
		{
			TryToInterrupt(NextUninterruptedInteractionId);
			TryToProgress();
		}
		else
		{
			CurrentMessageQueueEntry->AcceptResume(*MessageVisitor);
		}
		bIsPaused = false;
	}
}

void FCharacterMessageQueue::TryToProgress()
{
	bIsProgressing = true;

	bool bAdvancedQueue = false;
	while (!CurrentMessageQueueEntry.IsValid() && !bIsInterrupting)
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
		if (CurrentMessage->InteractionId == NextUninterruptedInteractionId)
		{
			NextUninterruptedInteractionId = {};
		}
		UE_LOG(LogInworldAIIntegration, Log, TEXT("Handle character message '%s::%s'"), *CurrentMessage->InteractionId, *CurrentMessage->UtteranceId);

		TSharedPtr<FCharacterMessageQueueLock> LockPinned = MakeLock();
		QueueLock = LockPinned;
		CurrentMessageQueueEntry->AcceptHandle(*MessageVisitor);

		bAdvancedQueue = true;
	}

	if (bAdvancedQueue && !NextUninterruptedInteractionId.IsEmpty())
	{
		TryToInterrupt(NextUninterruptedInteractionId);
	}

	bIsProgressing = false;
}

void FCharacterMessageQueue::OnUpdated(const FCharacterMessageTrigger& Message)
{
	const FString& InteractionId = Message.InteractionId;
	if (Message.Name == TEXT("inworld.uninterruptible"))
	{
		SetInterruptible(InteractionId, false);
	}
}

void FCharacterMessageQueue::OnUpdated(const FCharacterMessageInteractionEnd& Message)
{
	const FString& InteractionId = Message.InteractionId;
	if (!InteractionInterruptibleState.Contains(InteractionId))
	{
		SetInterruptible(InteractionId, true);
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
