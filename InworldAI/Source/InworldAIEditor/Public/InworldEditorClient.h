// Copyright 2023 Theai, Inc. (DBA Inworld) All Rights Reserved.

#pragma once

#include "InworldStudioTypes.h"

#include "InworldEditorClient.generated.h"

namespace Inworld
{
	class FEditorClient;
}

USTRUCT()
struct FInworldEditorClientOptions
{
public:
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Editor Client")
	FString ServerUrl;

	UPROPERTY(EditAnywhere, Category = "Editor Client")
	FString ExchangeToken;
};

USTRUCT()
struct INWORLDAIEDITOR_API FInworldEditorClient
{
public:
	GENERATED_BODY()

	void Init();
	void Destroy();

	void RequestFirebaseToken(const FInworldEditorClientOptions& Options, TFunction<void(const FString& FirebaseToken)> InCallback);
	void RequestReadyPlayerMeModelData(const FInworldStudioUserCharacterData& CharacterData, TFunction<void(const TArray<uint8>& Data)> InCallback);

	void CancelRequests();

	bool IsRequestInProgress() const;
	const FString& GetError() const;

private:
	TSharedPtr<Inworld::FEditorClient> InworldEditorClient;
};
