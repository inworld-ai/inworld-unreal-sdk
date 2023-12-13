// Copyright 2023 Theai, Inc. (DBA Inworld) All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "InworldRunnable.h"

THIRD_PARTY_INCLUDES_START
#include "AsyncRoutine.h"
#include "RunnableCommand.h"
THIRD_PARTY_INCLUDES_END

namespace Inworld
{
	class Runnable;
}

namespace Inworld
{
	class FAsyncRoutine
	{
	public:
		FAsyncRoutine(std::string InThreadName, std::unique_ptr<Inworld::Runnable> InRunnable)
			: RunnablePtr(MakeUnique<FInworldRunnable>(std::move(InRunnable)))
			, ThreadName(UTF8_TO_TCHAR(InThreadName.c_str()))
		{}

		virtual ~FAsyncRoutine() { Stop(); }

		void Start()
		{
			Thread.Reset(FRunnableThread::Create(RunnablePtr.Get(), *ThreadName));
		}

		void Stop()
		{
			if (Thread.IsValid())
			{
				Thread->Kill(true);
				Thread.Reset();
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
		TUniquePtr<FInworldRunnable> RunnablePtr;
		FString ThreadName = "";

	private:
		TUniquePtr<FRunnableThread> Thread;
	};

}

class FInworldAsyncRoutine : public Inworld::IAsyncRoutine
{
public:
	virtual void Start(std::string ThreadName, std::unique_ptr<Inworld::Runnable> Runnable) override
	{
		AsyncRoutinePtr = std::make_shared<Inworld::FAsyncRoutine>(ThreadName, std::move(Runnable));
		AsyncRoutinePtr->Start();
	}
	virtual void Stop() override { if (AsyncRoutinePtr) AsyncRoutinePtr->Stop(); }
	virtual bool IsDone() const { return AsyncRoutinePtr ? AsyncRoutinePtr->IsDone() : true; }
	virtual bool IsValid() const { return AsyncRoutinePtr ? AsyncRoutinePtr->IsValid() : false; }
	virtual Inworld::Runnable* GetRunnable() { return AsyncRoutinePtr ? AsyncRoutinePtr->GetRunnable() : nullptr; }

	std::shared_ptr<Inworld::FAsyncRoutine> AsyncRoutinePtr;
};
