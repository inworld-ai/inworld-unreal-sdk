/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "InworldCharacterPlayback.h"
#include "InworldCharacterMessage.h"

#include "InworldCharacterPlaybackText.generated.h"

UCLASS(BlueprintType, Blueprintable)
class INWORLDAIINTEGRATION_API UInworldCharacterPlaybackText : public UInworldCharacterPlayback
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInworldCharacterTextStart, const FString&, Id, bool, bIsPlayer);
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
	FOnInworldCharacterTextStart OnCharacterTextStart;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnInworldCharacterTextChanged, const FString&, Id, bool, bIsPlayer, const FString&, Text);
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
	FOnInworldCharacterTextChanged OnCharacterTextChanged;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnInworldCharacterTextFinal, const FString&, Id, bool, bIsPlayer, const FString&, Text);
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
	FOnInworldCharacterTextFinal OnCharacterTextFinal;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInworldCharacterTextInterrupt, const FString&, Id);
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
	FOnInworldCharacterTextInterrupt OnCharacterTextInterrupt;

protected:
	virtual void OnCharacterUtterance_Implementation(const FCharacterMessageUtterance& Message) override;
	virtual void OnCharacterUtteranceInterrupt_Implementation(const FCharacterMessageUtterance& Message) override;
	virtual void OnCharacterPlayerTalk_Implementation(const FCharacterMessagePlayerTalk& Message) override;
	virtual void OnCharacterInteractionEnd_Implementation(const FCharacterMessageInteractionEnd& Message) override;

private:
	void UpdateUtterance(const FString& InteractionId, const FString& UtteranceId, const FString& Text, bool bTextFinal, bool bIsPlayer);

	TMap<FString, TArray<FString>> InteractionIdToUtteranceIdMap;

	struct FInworldCharacterText
	{
		FInworldCharacterText() = default;
		FInworldCharacterText(const FString& InUtteranceId, const FString& InText, bool bInTextFinal, bool bInIsPlayer);

		FString Id;
		FString Text;
		bool bTextFinal = false;
		bool bIsPlayer = false;
	};

	TArray<FInworldCharacterText> CharacterTexts;
};
