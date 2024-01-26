/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "InworldTimer.h"
#include "Components/ActorComponent.h"

#include "InworldPlayerMultiTargetingComponent.generated.h"

class UInworldApiSubsystem;
class UInworldPlayerComponent;
class UInworldCharacterComponent;

UCLASS(ClassGroup = (Inworld), meta = (BlueprintSpawnableComponent))
class INWORLDAIINTEGRATION_API UInworldPlayerMultiTargetingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
    UInworldPlayerMultiTargetingComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void UpdateTargetCharacters();

public:
	/** Minimum distance to start interacting with a character */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	float InteractionDistance = 300.f;

private:
	TArray<UInworldCharacterComponent*> TargetCharacters;

	TWeakObjectPtr<UInworldApiSubsystem> InworldSubsystem;
	TWeakObjectPtr<UInworldPlayerComponent> PlayerComponent;

};
