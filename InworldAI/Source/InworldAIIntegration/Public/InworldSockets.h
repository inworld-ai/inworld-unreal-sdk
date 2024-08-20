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
class FInternetAddr;

namespace Inworld
{
	struct INWORLDAIINTEGRATION_API FSocketSettings
	{
		TSharedPtr<FInternetAddr> RemoteAddr;
		TSharedPtr<FInternetAddr> LocalAddr;
		FString Name;
		uint32 BufferSize;
	};

	class INWORLDAIINTEGRATION_API FSocketBase
	{
	public:
		FSocketBase(const FSocketSettings& Settings)
			: Socket(nullptr)
			, Settings(Settings)
		{}
		virtual ~FSocketBase();

		virtual bool ProcessData(TArray<uint8>& Data) = 0;
		const FSocketSettings& GetSettings() const { return Settings; }

	protected:
		FSocket* Socket;
		FSocketSettings Settings;
	}; 
	
	class INWORLDAIINTEGRATION_API FSocketSend : public FSocketBase
	{
	public:
		FSocketSend(const FSocketSettings& Settings);

		virtual bool ProcessData(TArray<uint8>& Data) override;
	};

	class INWORLDAIINTEGRATION_API FSocketReceive : public FSocketBase
	{
	public:
		FSocketReceive(const FSocketSettings& Settings);
		virtual ~FSocketReceive() override;
		
		virtual bool ProcessData(TArray<uint8>& Data) override;

	private:
		FUdpSocketReceiver* Receiver;
		TQueue<TArray<uint8>> DataQueue;
	};
}
