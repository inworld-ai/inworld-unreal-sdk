// Copyright 2023 Theai, Inc. (DBA Inworld) All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InworldCharacterPlayback.h"
#include "InworldCharacterMessage.h"

#include "InworldCharacterPlaybackHistory.generated.h"

USTRUCT(BlueprintType)
struct FInworldCharacterInteraction
{
	GENERATED_BODY();

	FInworldCharacterInteraction() = default;
	FInworldCharacterInteraction(const FString& InInteractionId, const FString& InUtteranceId, const FString& InText, bool bInPlayerInteraction)
		: InteractionId(InInteractionId)
		, UtteranceId(InUtteranceId)
		, Text(InText)
		, bPlayerInteraction(bInPlayerInteraction)
	{}

	FString InteractionId;
	FString UtteranceId;

	UPROPERTY(BlueprintReadOnly, Category = "Text")
	FString Text;

	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	bool bPlayerInteraction = false;
};

USTRUCT(BlueprintType)
struct FInworldCharacterInteractionHistory
{
	GENERATED_BODY();

	void Add(const FCharacterMessagePlayerTalk& Message) { Add(Message.InteractionId, Message.UtteranceId, Message.Text, true); }
	void Add(const FCharacterMessageUtterance& Message) { Add(Message.InteractionId, Message.UtteranceId, Message.Text, false); }
	void Add(const FString& InInteractionId, const FString& InUtteranceId, const FString& InText, bool bInPlayerInteraction);
	void Clear();

	void SetMaxEntries(uint32 Val);

	TArray<FInworldCharacterInteraction>& GetInteractions() { return Interactions; }

	void CancelUtterance(const FString& InteractionId, const FString& UtteranceId);
	bool IsInteractionCanceled(const FString& InteractionId) const;
	void ClearCanceledInteraction(const FString& InteractionId);

private:
	TArray<FInworldCharacterInteraction> Interactions;
	TArray<FString> CanceledInteractions;

	int32 MaxEntries = 50;
};

UCLASS(BlueprintType, Blueprintable)
class INWORLDAIINTEGRATION_API UInworldCharacterPlaybackHistory : public UInworldCharacterPlayback
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInworldCharacterInteractionsChanged, const TArray<FInworldCharacterInteraction>&, Interactions);
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
	FOnInworldCharacterInteractionsChanged OnInteractionsChanged;

	UFUNCTION(BlueprintPure, Category = "Interactions")
	const TArray<FInworldCharacterInteraction>& GetInteractions() { return InteractionHistory.GetInteractions(); }

	virtual void BeginPlay_Implementation() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (UIMin = "1", UIMax = "500"))
	int32 InteractionHistoryMaxEntries = 50;

private:
	virtual void OnCharacterUtterance_Implementation(const FCharacterMessageUtterance& Message) override;
	virtual void OnCharacterUtteranceInterrupt_Implementation(const FCharacterMessageUtterance& Message) override;
	virtual void OnCharacterPlayerTalk_Implementation(const FCharacterMessagePlayerTalk& Message) override;
	virtual void OnCharacterInteractionEnd_Implementation(const FCharacterMessageInteractionEnd& Message) override;

	FInworldCharacterInteractionHistory InteractionHistory;
};

