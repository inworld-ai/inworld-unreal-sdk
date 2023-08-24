/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"

#include "InworldRunnable.h"

namespace Inworld
{
	class Runnable;
}

namespace Inworld
{
	template<typename T>
	class FAsyncRoutine
	{
	public:
		virtual ~FAsyncRoutine() { Stop(); }

		void Start()
		{
			Thread = FRunnableThread::Create(RunnablePtr.Get(), *ThreadName);
		}

		void Stop()
		{
			if (Thread)
			{
				Thread->Kill(true);
				delete Thread;
				Thread = nullptr;
			}

			if (RunnablePtr)
			{
				RunnablePtr->Stop();
				RunnablePtr.Reset();
			}
		}

		bool IsValid() const { return RunnablePtr && RunnablePtr->IsValid() && Thread; }
		bool IsDone() const { return RunnablePtr && RunnablePtr->IsValid() && RunnablePtr->IsDone(); }

		Inworld::Runnable* GetRunnable() { return RunnablePtr ? RunnablePtr->GetTask() : nullptr; }

	protected:
		TUniquePtr<T> RunnablePtr;
		FString ThreadName = "";

	private:
		FRunnableThread* Thread = nullptr;
	};

}

struct INWORLDAICLIENT_API FInworldAsyncRoutine
{
public:

	FInworldAsyncRoutine() = default;
	FInworldAsyncRoutine(FString InThreadName, TUniquePtr<Inworld::Runnable> InRunnable)
		: AsyncRoutinePtr(MakeShared<AsyncRoutine>(InThreadName, MoveTemp(InRunnable)))
	{}

	void Start() { AsyncRoutinePtr->Start(); }
	void Stop() { AsyncRoutinePtr->Stop(); }
	bool IsDone() { return AsyncRoutinePtr->IsDone(); }

private:
	struct Runnable : public FInworldRunnable<TUniquePtr<Inworld::Runnable>>
	{
		Runnable(TUniquePtr<Inworld::Runnable> InRunnable)
		{
			RunnablePtr = MoveTemp(InRunnable);
		}
	};

	struct AsyncRoutine : public Inworld::FAsyncRoutine<Runnable>
	{
		AsyncRoutine(FString InThreadName, TUniquePtr<Inworld::Runnable> InRunnable)
		{
			ThreadName = InThreadName;
			RunnablePtr = MakeUnique<Runnable>(MoveTemp(InRunnable));
		}
	};

	TSharedPtr<AsyncRoutine> AsyncRoutinePtr;
};
