/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "InworldStudioTypes.h"

#include "InworldStudio.generated.h"

namespace Inworld
{
	class FStudio;
}

USTRUCT()
struct INWORLDAICLIENT_API FInworldStudio
{
public:
	GENERATED_BODY()

	FInworldStudio();
	~FInworldStudio();

	void RequestStudioUserData(const FString& Token, const FString& ServerUrl, TFunction<void(bool bSuccess)> InCallback);

	void CancelRequests();
	bool IsRequestInProgress() const;

	FString GetError() const;
	FInworldStudioUserData GetStudioUserData() const;

private:
	mutable FInworldStudioUserData Data;
};
