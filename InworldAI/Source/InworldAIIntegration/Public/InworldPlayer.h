/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "UObject/NoExportTypes.h"
#include "InworldPlayer.generated.h"

class UInworldSession;
class UInworldCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInworldPlayerTargetCharacterAdded, UInworldCharacter*, Character);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInworldPlayerTargetCharacterAddedNative, UInworldCharacter* /*Character*/);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInworldPlayerTargetCharacterRemoved, UInworldCharacter*, Character);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInworldPlayerTargetCharacterRemovedNative, UInworldCharacter* /*Character*/);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInworldPlayerTargetCharactersChanged);
DECLARE_MULTICAST_DELEGATE(FOnInworldPlayerTargetCharactersChangedNative);

UCLASS(BlueprintType)
class INWORLDAIINTEGRATION_API UInworldPlayer : public UObject
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Inworld|Player")
	TScriptInterface<IInworldPlayerOwnerInterface> GetInworldPlayerOwner();

	UFUNCTION(BlueprintCallable, Category = "Inworld|Player|Target")
	const TArray<UInworldCharacter*>& GetTargetCharacters() const { return TargetCharacters; }

	UFUNCTION(BlueprintCallable, Category = "Inworld|Player|Target")
	void AddTargetCharacter(UInworldCharacter* TargetCharacter);

	UFUNCTION(BlueprintCallable, Category = "Inworld|Player|Target")
	void RemoveTargetCharacter(UInworldCharacter* TargetCharacter);

	UFUNCTION(BlueprintCallable, Category = "Inworld|Player|Target")
	void ClearAllTargetCharacters();

	UPROPERTY(BlueprintAssignable, Category = "Engagement")
	FOnInworldPlayerTargetCharacterAdded OnTargetCharacterAddedDelegate;
	FOnInworldPlayerTargetCharacterAddedNative& OnTargetCharacterAdded() { return OnTargetCharacterAddedDelegateNative; }

	UPROPERTY(BlueprintAssignable, Category = "Engagement")
	FOnInworldPlayerTargetCharacterRemoved OnTargetCharacterRemovedDelegate;
	FOnInworldPlayerTargetCharacterRemovedNative& OnTargetCharacterRemoved() { return OnTargetCharacterRemovedDelegateNative; }

	UPROPERTY(BlueprintAssignable, Category = "Engagement")
	FOnInworldPlayerTargetCharactersChanged OnTargetCharactersChangedDelegate;
	FOnInworldPlayerTargetCharactersChangedNative& OnTargetCharactersChanged() { return OnTargetCharactersChangedDelegateNative; }

private:
	UPROPERTY()
	TArray<UInworldCharacter*> TargetCharacters;

	FOnInworldPlayerTargetCharacterAddedNative OnTargetCharacterAddedDelegateNative;
	FOnInworldPlayerTargetCharacterRemovedNative OnTargetCharacterRemovedDelegateNative;
	FOnInworldPlayerTargetCharactersChangedNative OnTargetCharactersChangedDelegateNative;
};

UINTERFACE(MinimalAPI, BlueprintType)
class UInworldPlayerOwnerInterface : public UInterface
{
	GENERATED_BODY()
};

class INWORLDAIINTEGRATION_API IInworldPlayerOwnerInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inworld")
	UInworldSession* GetInworldSession() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inworld")
	UInworldPlayer* GetInworldPlayer() const;

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inworld")
	FTransform GetInworldPlayerTransform() const;
};