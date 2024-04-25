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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInworldCharacterPossessed, bool, bPossessed);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInworldCharacterPossessedNative, bool /*bPossessed*/);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInworldCharacterTargetPlayerChanged);
DECLARE_MULTICAST_DELEGATE(FOnInworldCharacterTargetPlayerChangedNative);

UCLASS(BlueprintType)
class INWORLDAIINTEGRATION_API UInworldCharacter : public UObject
{
	GENERATED_BODY()
public:
	//UObject
	virtual UWorld* GetWorld() const override { return GetTypedOuter<AActor>()->GetWorld(); }
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual int32 GetFunctionCallspace(UFunction* Function, FFrame* Stack) override;
	virtual bool CallRemoteFunction(UFunction* Function, void* Parms, struct FOutParmRec* OutParms, FFrame* Stack) override;
	//~UObject

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
	FOnInworldCharacterPossessed OnPossessedDelegate;
	FOnInworldCharacterPossessedNative& OnPossessed() { return OnPossessedDelegateNative; }

	UFUNCTION(BlueprintCallable, Category = "Engagement")
	void SetTargetPlayer(UInworldPlayer* Player);
	UFUNCTION(BlueprintCallable, Category = "Engagement")
	void ClearTargetPlayer();

	UFUNCTION(BlueprintPure, Category="Engagement")
	UInworldPlayer* GetTargetPlayer() const { return TargetPlayer; }

	UPROPERTY(BlueprintAssignable, Category = "Engagement")
	FOnInworldCharacterTargetPlayerChanged OnTargetPlayerChangedDelegate;
	FOnInworldCharacterTargetPlayerChangedNative& OnTargetPlayerChanged() { return OnTargetPlayerChangedDelegateNative; }

private:
	UPROPERTY(Replicated)
	FInworldAgentInfo AgentInfo;
	FOnInworldCharacterPossessedNative OnPossessedDelegateNative;

	UFUNCTION()
	void OnRep_TargetPlayer();

	UPROPERTY(ReplicatedUsing=OnRep_TargetPlayer)
	UInworldPlayer* TargetPlayer;
	FOnInworldCharacterTargetPlayerChangedNative OnTargetPlayerChangedDelegateNative;
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
};

