/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

THIRD_PARTY_INCLUDES_START
#include "Runnable.h"
THIRD_PARTY_INCLUDES_END

#include "HAL/RunnableThread.h"
#include "HAL/Runnable.h"

class FInworldRunnable : public FRunnable
{
public:
	FInworldRunnable(std::unique_ptr<Inworld::Runnable> InRunnablePtr)
		: RunnablePtr(std::move(InRunnablePtr))
	{}

	bool IsDone() const { return RunnablePtr ? RunnablePtr->IsDone() : false; };
	bool IsValid() const { return GetTask() != nullptr; }

	Inworld::Runnable* GetTask() const { return RunnablePtr.get(); }

	virtual uint32 Run() override
	{
		if (ensure(RunnablePtr))
		{
			RunnablePtr->Run();
		}

		return 0;
	}

	virtual void Stop() override
	{
		if (RunnablePtr)
		{
			RunnablePtr->Stop();
		}
	}

protected:
	std::unique_ptr<Inworld::Runnable> RunnablePtr;
};
