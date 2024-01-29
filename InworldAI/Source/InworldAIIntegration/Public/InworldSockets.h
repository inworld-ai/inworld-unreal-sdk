/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"

#include "Containers/Queue.h"

class FSocket;
class FUdpSocketReceiver;

namespace Inworld
{
	struct INWORLDAIINTEGRATION_API FSocketSettings
	{
		FString IpAddr;
		FString Name;
		uint32 Port;
		uint32 BufferSize;
	};

	class INWORLDAIINTEGRATION_API FSocketBase
	{
	public:
		virtual ~FSocketBase() = default;

		virtual bool Initialize(const FSocketSettings& Settings) = 0;
		virtual bool Deinitialize() = 0;
		virtual bool ProcessData(TArray<uint8>& Data) = 0;

	protected:
		FSocket* Socket;
	}; 
	
	class INWORLDAIINTEGRATION_API FSocketSend : public FSocketBase
	{
	public:
		virtual ~FSocketSend() = default;

		virtual bool Initialize(const FSocketSettings& Settings) override;
		virtual bool Deinitialize() override;
		virtual bool ProcessData(TArray<uint8>& Data) override;
	};

	class INWORLDAIINTEGRATION_API FSocketReceive : public FSocketBase
	{
	public:
		virtual ~FSocketReceive() = default;

		virtual bool Initialize(const FSocketSettings& Settings) override;
		virtual bool Deinitialize() override;
		virtual bool ProcessData(TArray<uint8>& Data) override;

	private:
		FUdpSocketReceiver* Receiver;
		
		TQueue<TArray<uint8>> DataQueue;
	};
}
