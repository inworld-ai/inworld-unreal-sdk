/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once
#include <thread>
#include <atomic>

#include "RunnableCommand.h"

namespace Inworld
{
	class IAsyncRoutine
	{
	public:
		virtual ~IAsyncRoutine() = default;

		virtual void Start(std::string ThreadName, std::unique_ptr<Runnable> Runnable) = 0;
		virtual void Stop() = 0;
		virtual bool IsDone() const = 0;
		virtual bool IsValid() const = 0;
		virtual Inworld::Runnable* GetRunnable() = 0;
	};

	class AsyncRoutine : public IAsyncRoutine
	{
	public:
		virtual ~AsyncRoutine() = default;

		virtual void Start(std::string ThreadName, std::unique_ptr<Runnable> Runnable) override;
		virtual void Stop() override;
		
		virtual bool IsDone() const override
		{
			return _Runnable ? _Runnable->IsDone() : false;
		}
		
		virtual bool IsValid() const override
		{ 
			return _Runnable && _Thread;
		}

		virtual Runnable* GetRunnable() override
		{
			return _Runnable.get();
		}

	protected:
		std::unique_ptr<Runnable> _Runnable;
		std::unique_ptr<std::thread> _Thread;
	};

}