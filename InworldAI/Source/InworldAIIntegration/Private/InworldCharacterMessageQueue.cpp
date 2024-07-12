/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */
#include "InworldCharacterMessageQueue.h"
#include "InworldAIIntegrationModule.h"

void FCharacterMessageQueue::TryToPause()
{
	if (!bIsPaused && CanPauseCurrentMessageQueueEntry())
	{
		if (CurrentMessageQueueEntry.IsValid())
		{
			CurrentMessageQueueEntry->AcceptPause(*MessageVisitor);
		}
	}
	bIsPaused = true;
}

void FCharacterMessageQueue::TryToResume()
{
	if (bIsPaused && !bIsPendingInterruptState)
	{
		if (CurrentMessageQueueEntry.IsValid())
		{
			CurrentMessageQueueEntry->AcceptResume(*MessageVisitor);
		}
		TryToProgress();
	}
	bIsPaused = false;
}

void FCharacterMessageQueue::TryToInterrupt(const FString& InterruptingInteractionId)
{
	if (InterruptingInteractionId == NextInterruptingInteractionId)
	{
		return;
	}

	bIsInterrupting = true;

	NextInterruptingInteractionId = InterruptingInteractionId;

	switch (GetQueueEntryInterruptibleState(CurrentMessageQueueEntry))
	{
	case EInworldInteractionInterruptibleState::UNDETERMINED:
		CurrentMessageQueueEntry->AcceptPause(*MessageVisitor);
		bIsPendingInterruptState = true;
		break;
	case EInworldInteractionInterruptibleState::YES:
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
		break;
	}

	if (!CurrentMessageQueueEntry.IsValid())
	{
		CancelInterruptiblePendingQueueEntries();

		if (!bIsProgressing)
		{
			TryToProgress();
		}
	}

	bIsInterrupting = false;
}

void FCharacterMessageQueue::TryToProgress()
{
	bIsProgressing = true;

	bool bAdvancedQueue = false;
	while (!CurrentMessageQueueEntry.IsValid() && !bIsInterrupting)
	{
		if (PendingMessageQueueEntries.Num() == 0)
		{
			break;
		}

		auto NextQueuedEntry = PendingMessageQueueEntries[0];
		if(!NextQueuedEntry->IsReady())
		{
			break;
		}

		CurrentMessageQueueEntry = NextQueuedEntry;

		if (bIsPaused && CanPauseCurrentMessageQueueEntry())
		{
			CurrentMessageQueueEntry = nullptr;
			break;
		}

		PendingMessageQueueEntries.RemoveAt(0);

		auto CurrentMessage = CurrentMessageQueueEntry->GetCharacterMessage();
		UE_LOG(LogInworldAIIntegration, Log, TEXT("Handle character message '%s::%s'"), *CurrentMessage->InteractionId, *CurrentMessage->UtteranceId);

		TSharedPtr<FCharacterMessageQueueLock> LockPinned = MakeLock();
		QueueLock = LockPinned;
		CurrentMessageQueueEntry->AcceptHandle(*MessageVisitor);

		bAdvancedQueue = true;
	}

	if (bAdvancedQueue && NextInterruptingInteractionId.IsSet())
	{
		TryToInterrupt(NextInterruptingInteractionId.GetValue());
	}

	bIsProgressing = false;
}

void FCharacterMessageQueue::SetInterruptible(const FString& InteractionId, bool bInterruptible)
{
	InteractionInterruptibleState.Add(InteractionId, bInterruptible);
	if (bIsPendingInterruptState && CurrentMessageQueueEntry->GetCharacterMessage()->InteractionId == InteractionId)
	{
		if (bInterruptible)
		{
			TryToInterrupt(NextInterruptingInteractionId.GetValue());
			TryToProgress();
		}
		else
		{
			CurrentMessageQueueEntry->AcceptResume(*MessageVisitor);
		}
		bIsPendingInterruptState = false;
	}
}

bool FCharacterMessageQueue::CanPauseCurrentMessageQueueEntry() const
{
	const EInworldInteractionInterruptibleState CurrentInterruptibleState = GetQueueEntryInterruptibleState(CurrentMessageQueueEntry);
	return CurrentInterruptibleState == EInworldInteractionInterruptibleState::YES || CurrentInterruptibleState == EInworldInteractionInterruptibleState::UNDETERMINED;
}

void FCharacterMessageQueue::CancelInterruptiblePendingQueueEntries()
{
	while (PendingMessageQueueEntries.Num() > 0 && GetQueueEntryInterruptibleState(PendingMessageQueueEntries[0]) == EInworldInteractionInterruptibleState::YES)
	{
		PendingMessageQueueEntries[0]->AcceptCancel(*MessageVisitor);
		PendingMessageQueueEntries.RemoveAt(0);
	}
}

FCharacterMessageQueue::EInworldInteractionInterruptibleState FCharacterMessageQueue::GetInteractionInterruptibleState(const FString& InteractionId) const
{
	if (InteractionInterruptibleState.Contains(InteractionId))
	{
		return InteractionInterruptibleState[InteractionId] ? EInworldInteractionInterruptibleState::YES : EInworldInteractionInterruptibleState::NO;
	}
	return EInworldInteractionInterruptibleState::UNDETERMINED;
}

FCharacterMessageQueue::EInworldInteractionInterruptibleState FCharacterMessageQueue::GetQueueEntryInterruptibleState(const TSharedPtr<FCharacterMessageQueueEntryBase>& QueueEntry) const
{
	if (!QueueEntry.IsValid() || !QueueEntry->GetCharacterMessage().IsValid())
	{
		return EInworldInteractionInterruptibleState::INVALID;
	}
	return GetInteractionInterruptibleState(QueueEntry->GetCharacterMessage()->InteractionId);
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

void FCharacterMessageQueue::EndInteraction(const FString& InteractionId)
{
	if (InteractionId == NextInterruptingInteractionId)
	{
		NextInterruptingInteractionId.Reset();
	}
	if (!InteractionInterruptibleState[InteractionId])
	{
		CancelInterruptiblePendingQueueEntries();
	}
	InteractionInterruptibleState.Remove(InteractionId);
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
		auto CurrentMessageQueueEntry = QueuePinned->CurrentMessageQueueEntry;
		if (CurrentMessageQueueEntry->IsEnd())
		{
			QueuePinned->EndInteraction(CurrentMessageQueueEntry->GetCharacterMessage()->InteractionId);
		}
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
