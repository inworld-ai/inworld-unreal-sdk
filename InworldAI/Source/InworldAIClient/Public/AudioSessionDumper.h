/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"

#if !UE_BUILD_SHIPPING

#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"
#include "HAL/PlatformProcess.h"

#include "Containers/Queue.h"

class FAudioSessionDumper
{
public:
	void OnSessionStart(const FString& InFileName);
	void OnSessionStop();
	void OnMessage(const TArray<uint8>& Msg);

private:
	FString FileName;
};

class FAudioDumperRunnable : public FRunnable
{
public:
	FAudioDumperRunnable(const FString& InFileName, TQueue<TArray<uint8>>& InAudioChunks);

	uint32 Run() override;

	void Stop() override { bIsDone = true; }

private:
	FString FileName;
	TQueue<TArray<uint8>>& AudioChunks;
	FAudioSessionDumper AudioDumper;

	bool bIsDone = false;
};

class FAsyncAudioDumper
{
public:
	FAsyncAudioDumper() = default;
	~FAsyncAudioDumper();

	void Start(const FString& Filename);
	void Stop();

	void QueueChunk(const TArray<uint8> Chunk) { AudioChunksToDump.Enqueue(Chunk); }

private:
	FRunnableThread* Thread;
	TUniquePtr<FRunnable> Runnable;

	TQueue<TArray<uint8>> AudioChunksToDump;
};

#endif
