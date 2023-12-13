// Copyright 2023 Theai, Inc. (DBA Inworld) All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InworldTimer.h"
#include "Components/ActorComponent.h"

#include "InworldPlayerTargetingComponent.generated.h"

class UInworldApiSubsystem;
class UInworldPlayerComponent;
class UInworldCharacterComponent;

UCLASS(ClassGroup = (Inworld), meta = (BlueprintSpawnableComponent))
class INWORLDAIINTEGRATION_API UInworldPlayerTargetingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
    UInworldPlayerTargetingComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    void UpdateTargetCharacter();
    void SetTargetCharacter(TWeakObjectPtr<UInworldCharacterComponent> ClosestCharacter);
    void ClearTargetCharacter();

public:
	/** Minimum distance to start interacting with a character */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	float InteractionDistance = 300.f;

private:

    /** How close must player be facing the direction of the character to interact */
    UPROPERTY(EditAnywhere, Category = "Interaction")
	float InteractionDotThreshold = 0.5f;

	TWeakObjectPtr<UInworldApiSubsystem> InworldSubsystem;
	TWeakObjectPtr<UInworldPlayerComponent> PlayerComponent;

    Inworld::Utils::FWorldTimer ChangeTargetCharacterTimer = Inworld::Utils::FWorldTimer(0.5f);

};
