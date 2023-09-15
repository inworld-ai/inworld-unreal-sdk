/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */
#include "InworldCharacterMessage.h"
#include "InworldAIIntegrationModule.h"

TArray<FString> FCharacterMessageQueue::CancelInteraction(const FString& InteractionId)
{
	TArray<FString> CanceledUtterances;
	CanceledUtterances.Reserve(PendingMessageEntries.Num() + 1);

	if (CurrentMessage.IsValid() && CurrentMessage->InteractionId == InteractionId)
	{
		CanceledUtterances.Add(CurrentMessage->UtteranceId);
		CurrentMessage->AcceptInterrupt(*MessageVisitor);
		CurrentMessage = nullptr;
		LockCount = 0;
	}

	TArray<FCharacterMessageQueueEntry> RemainingMessageEntries;
	RemainingMessageEntries.Reserve(PendingMessageEntries.Num());
	for (auto& PendingMessageEntry : PendingMessageEntries)
	{
		auto& PendingMessage = PendingMessageEntry.Message;
		if (InteractionId == PendingMessage->InteractionId)
		{
			CanceledUtterances.Add(PendingMessage->UtteranceId);

			PendingMessage->AcceptCancel(*MessageVisitor);
		}
		else
		{
			RemainingMessageEntries.Add(PendingMessageEntry);
		}
	}

	PendingMessageEntries = RemainingMessageEntries;

	TryToProgress();

	return CanceledUtterances;
}

void FCharacterMessageQueue::TryToProgress(bool bForce)
{
	while (!CurrentMessage.IsValid() || LockCount == 0)
	{
		CurrentMessage = nullptr;

		if (PendingMessageEntries.Num() == 0)
		{
			return;
		}

		auto NextQueuedEntry = PendingMessageEntries[0];
		if(!NextQueuedEntry.Message->IsReady() && !bForce)
		{
			return;
		}

		CurrentMessage = NextQueuedEntry.Message;
		PendingMessageEntries.RemoveAt(0);

		UE_LOG(LogInworldAIIntegration, Log, TEXT("Handle character message '%s::%s'"), *CurrentMessage->InteractionId, *CurrentMessage->UtteranceId);

		CurrentMessage->AcceptHandle(*MessageVisitor);
	}
}

TOptional<float> FCharacterMessageQueue::GetBlockingTimestamp() const
{
	TOptional<float> Timestamp;
	if (!CurrentMessage.IsValid() && PendingMessageEntries.Num() > 0)
	{
		auto NextQueuedEntry = PendingMessageEntries[0];
		if (!NextQueuedEntry.Message->IsReady())
		{
			Timestamp = NextQueuedEntry.Timestamp;
		}
	}
	return Timestamp;
}

void FCharacterMessageQueue::Clear()
{
	LockCount = 0;

	if (CurrentMessage)
	{
		CurrentMessage->AcceptInterrupt(*MessageVisitor);
		CurrentMessage = nullptr;
	}

	PendingMessageEntries.Empty();
}

TSharedPtr<FCharacterMessageQueueLock> FCharacterMessageQueue::MakeLock()
{
	LockCount++;
	return MakeShared<FCharacterMessageQueueLock>(AsShared());
}

FCharacterMessageQueueLock::FCharacterMessageQueueLock(TSharedRef<FCharacterMessageQueue> InQueue)
	: QueuePtr(InQueue)
	, MessagePtr(InQueue->CurrentMessage)
{}

FCharacterMessageQueueLock::~FCharacterMessageQueueLock()
{
	auto Queue = QueuePtr.Pin();
	auto Message = MessagePtr.Pin();
	if (Queue && Message)
	{
		if (Queue->CurrentMessage == Message)
		{
			Queue->LockCount--;
			if (Queue->LockCount == 0)
			{
				Queue->TryToProgress();
			}
		}
	}
}
