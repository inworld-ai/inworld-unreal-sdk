/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "InworldTimer.h"
#include "Components/ActorComponent.h"

#include "InworldPlayerTargetingComponent.generated.h"

class UInworldApiSubsystem;
class UInworldPlayer;
class UInworldCharacter;

UCLASS(ClassGroup = (Inworld), meta = (BlueprintSpawnableComponent))
class INWORLDAIINTEGRATION_API UInworldPlayerTargetingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInworldPlayerTargetingComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void UpdateTargetCharacters();

public:
	/** Minimum distance to start interacting with a character */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	float InteractionDistance = 300.f;

	/** Enable multiple targets */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	bool bMultipleTargets = false;

private:
	TWeakObjectPtr<UInworldPlayer> InworldPlayer;
	TArray<TWeakObjectPtr<UInworldCharacter>> TargetCharacters;
};
