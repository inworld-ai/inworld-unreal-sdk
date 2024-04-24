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
	void AddTargetCharacter(UInworldCharacter* TargetCharacter) { TargetCharacters.AddUnique(TargetCharacter); }

	UFUNCTION(BlueprintCallable, Category = "Inworld|Player|Target")
	void RemoveTargetCharacter(UInworldCharacter* TargetCharacter) { TargetCharacters.Remove(TargetCharacter); }

	UFUNCTION(BlueprintCallable, Category = "Inworld|Player|Target")
	void ClearAllTargetCharacters() { TargetCharacters.Empty(); }

private:
	UPROPERTY()
	TArray<UInworldCharacter*> TargetCharacters;
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