/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "HAL/RunnableThread.h"
#include "HAL/Runnable.h"

namespace Inworld
{
	class Runnable;
}

template<typename T> 
class FInworldRunnable : public FRunnable
{
public:
	bool IsDone() const { return RunnablePtr ? RunnablePtr->IsDone() : false; };
	bool IsValid() const { return GetTask() != nullptr; }

	Inworld::Runnable* GetTask() const { return RunnablePtr.operator->(); }

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
	T RunnablePtr;
};
