// Copyright 2023 Theai, Inc. (DBA Inworld) All Rights Reserved.

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

	const FString& GetError() const;
	FInworldStudioUserData GetStudioUserData() const;

private:

	TSharedPtr<Inworld::FStudio> InworldStudio;
};
