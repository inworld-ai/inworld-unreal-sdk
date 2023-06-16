/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "HAL/RunnableThread.h"
#include "HAL/Runnable.h"

#include "NDK/RunnableCommand.h"
#include "NDK/AsyncRoutine.h"

namespace Inworld
{
	class INWORLDAICLIENT_API FInworldRunnable : public FRunnable
	{
	public:
		FInworldRunnable(std::unique_ptr<Inworld::Runnable> InRunnable)
			: Runnable(std::move(InRunnable))
		{}

		bool IsDone() const { return Runnable ? Runnable->IsDone() : false; };
		bool IsValid() const { return Runnable.get() != nullptr; }

		Inworld::Runnable* GetTask() const { return Runnable.get(); }

		virtual uint32 Run() override
		{
			if (ensure(Runnable))
			{
				Runnable->Run();
			}

			return 0;
		}

		virtual void Stop() override 
		{
			if (Runnable)
			{
				Runnable->Stop();
			}
		}

	protected:
		std::unique_ptr<Inworld::Runnable> Runnable;
	};

    class FAsyncRoutine : public Inworld::IAsyncRoutine
    {
    public:
		~FAsyncRoutine() { Stop(); }

		virtual void Start(std::string ThreadName, std::unique_ptr<Inworld::Runnable> InRunnable) override
		{
			Stop();
			Runnable = std::make_unique<FInworldRunnable>(std::move(InRunnable));
			Thread = FRunnableThread::Create(Runnable.get(), UTF8_TO_TCHAR(ThreadName.c_str()));
		}

		virtual void Stop() override
        {
            if (Thread)
            {
                Thread->Kill(true);
				delete Thread;
				Thread = nullptr;
            }

			if (Runnable)
			{
				Runnable->Stop();
				Runnable.release();
			}
        }

		virtual bool IsValid() const override { return Runnable && Runnable->IsValid() && Thread; }
		virtual bool IsDone() const override { return Runnable && Runnable->IsValid() && Runnable->IsDone(); }

		virtual Inworld::Runnable* GetRunnable() override { return Runnable ? Runnable->GetTask() : nullptr; }

    private:
		FRunnableThread* Thread = nullptr;
		std::unique_ptr<FInworldRunnable> Runnable;
    };
}
