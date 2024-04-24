/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "InworldTypes.h"
#include "InworldCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInworldPlayerPossessed, bool, bPossessed);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInworldPlayerPossessedNative, bool /*bPossessed*/);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInworldPlayerEngaged, bool, bEngaged);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInworldPlayerEngagedNative, bool /*bEngaged*/);

UCLASS(BlueprintType)
class INWORLDAIINTEGRATION_API UInworldCharacter : public UObject
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Inworld|Player")
	TScriptInterface<IInworldCharacterOwnerInterface> GetInworldCharacterOwner();

	UFUNCTION(BlueprintCallable, Category = "Inworld|Player")
	void SetBrainName(const FString& BrainName);

	UFUNCTION(BlueprintPure, Category = "Inworld|Player")
	bool IsPossessed() const { return !AgentInfo.AgentId.IsEmpty(); }
	UFUNCTION(BlueprintCallable, Category = "Inworld|Player")
	void Possess(const FInworldAgentInfo& InAgentInfo);
	UFUNCTION(BlueprintCallable, Category = "Inworld|Player")
	void Unpossess();

	UFUNCTION(BlueprintPure, Category = "Inworld|Agent")
	const FInworldAgentInfo& GetAgentInfo() const { return AgentInfo; }

	UPROPERTY(BlueprintAssignable, Category = "Possession")
	FOnInworldPlayerPossessed OnPossessedDelegate;
	FOnInworldPlayerPossessedNative& OnPossessed() { return OnPossessedDelegateNative; }

	UFUNCTION(BlueprintPure, Category = "Engagement")
	bool IsEngagedWithPlayer() const { return EngagedPlayer != nullptr; }
	UFUNCTION(BlueprintCallable, Category = "Engagement")
	void SetEngagedPlayer(UInworldPlayer* Player);
	UFUNCTION(BlueprintCallable, Category = "Engagement")
	void ClearEngagedPlayer();

	UFUNCTION(BlueprintPure, Category="Engagement")
	UInworldPlayer* GetEngagedPlayer() const { return EngagedPlayer; }

	UPROPERTY(BlueprintAssignable, Category = "Engagement")
	FOnInworldPlayerEngaged OnEngagedDelegate;
	FOnInworldPlayerEngagedNative& OnEngaged() { return OnEngagedDelegateNative; }

private:
	FInworldAgentInfo AgentInfo;
	FOnInworldPlayerPossessedNative OnPossessedDelegateNative;

	UPROPERTY()
	UInworldPlayer* EngagedPlayer;
	FOnInworldPlayerEngagedNative OnEngagedDelegateNative;
};


UINTERFACE(MinimalAPI, BlueprintType)
class UInworldCharacterOwnerInterface : public UInterface
{
	GENERATED_BODY()
};

class INWORLDAIINTEGRATION_API IInworldCharacterOwnerInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inworld")
	UInworldSession* GetInworldSession() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inworld")
	UInworldCharacter* GetInworldCharacter() const;

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inworld")
	FTransform GetInworldCharacterTransform() const;
};

