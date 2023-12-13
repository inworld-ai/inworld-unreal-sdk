// Copyright 2023 Theai, Inc. (DBA Inworld) All Rights Reserved.

#pragma once

THIRD_PARTY_INCLUDES_START
#include "RunnableCommand.h"
THIRD_PARTY_INCLUDES_END

#include "HAL/RunnableThread.h"
#include "HAL/Runnable.h"

namespace Inworld
{
	class Runnable;
}

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
