/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "AsyncRoutine.h"



void Inworld::AsyncRoutine::Start(std::string ThreadName, std::unique_ptr<Runnable> Runnable)
{
	Stop();
	_Runnable = std::move(Runnable);
	_Thread = std::make_unique<std::thread>(&Runnable::Run, _Runnable.get());
}

void Inworld::AsyncRoutine::Stop()
{
	if (_Runnable)
	{
		_Runnable->Stop();
		_Runnable.release();
	}

	if (_Thread)
	{
		_Thread.release();
	}
}
