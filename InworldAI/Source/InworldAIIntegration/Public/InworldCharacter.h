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


UCLASS(BlueprintType)
class INWORLDAIINTEGRATION_API UInworldCharacter : public UObject
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Inworld|Player")
	TScriptInterface<IInworldCharacterOwnerInterface> GetInworldCharacterOwner();

	void SetBrainName(const FString& BrainName);

	void Possess(const FInworldAgentInfo& InAgentInfo);
	void Unpossess();

	UFUNCTION(BlueprintPure, Category = "Inworld|Agent")
	const FString& GetBrainName() const { return AgentInfo.BrainName; }
	UFUNCTION(BlueprintPure, Category = "Inworld|Agent")
	const FString& GetAgentId() const { return AgentInfo.AgentId; }
	UFUNCTION(BlueprintPure, Category = "Inworld|Agent")
	const FString& GetGivenName() const { return AgentInfo.GivenName; }

private:
	FInworldAgentInfo AgentInfo;
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

