/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldPlayerTargetingComponent.h"
#include "InworldPlayerComponent.h"
#include "Camera/CameraComponent.h"


UInworldPlayerTargetingComponent::UInworldPlayerTargetingComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UInworldPlayerTargetingComponent::BeginPlay()
{
    Super::BeginPlay();

    if (GetOwnerRole() != ROLE_Authority)
    {
        PrimaryComponentTick.SetTickFunctionEnable(false);
    }
    else
	{
		PrimaryComponentTick.SetTickFunctionEnable(true);

        InworldSubsystem = GetWorld()->GetSubsystem<UInworldApiSubsystem>();
        PlayerComponent = Cast<UInworldPlayerComponent>(GetOwner()->GetComponentByClass(UInworldPlayerComponent::StaticClass()));
    }
}

void UInworldPlayerTargetingComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdateTargetCharacter();
}

void UInworldPlayerTargetingComponent::UpdateTargetCharacter()
{
    const auto& CharacterComponents = InworldSubsystem->GetCharacterComponents();
    TWeakObjectPtr<UInworldCharacterComponent> ClosestCharacter;
    const float MinDistSq = InteractionDistance * InteractionDistance;
    float CurMaxDot = -1.f;
    const FVector Location = GetOwner()->GetActorLocation();
    for (auto& Character : CharacterComponents)
    {
        if (!Character || Character->GetAgentId().IsEmpty())
        {
            continue;
        }

        Inworld::IPlayerComponent* CurrentInteractonPlayer = Character->GetTargetPlayer();
        if (CurrentInteractonPlayer && CurrentInteractonPlayer != PlayerComponent.Get())
        {
            continue;
        }

        const FVector CharacterLocation = Character->GetComponentOwner()->GetActorLocation();
        const float DistSq = FVector::DistSquared(Location, CharacterLocation);
        if (DistSq > MinDistSq)
        {
            continue;
        }

        const FVector2D Direction2D = FVector2D(CharacterLocation - Location).GetSafeNormal();
        FVector2D Forward2D;
        if (auto* CameraComponent = Cast<UCameraComponent>(GetOwner()->GetComponentByClass(UCameraComponent::StaticClass())))
        {
            Forward2D = FVector2D(CameraComponent->K2_GetComponentRotation().Vector());
        }
        else
        {
            Forward2D = FVector2D(GetOwner()->GetActorRotation().Vector());
        }

        const float Dot = FVector2D::DotProduct(Forward2D, Direction2D);
        if (Dot < InteractionDotThreshold)
        {
            continue;
        }

        if (Dot < CurMaxDot)
        {
            continue;
        }

        ClosestCharacter = static_cast<UInworldCharacterComponent*>(Character);
        CurMaxDot = Dot;
    }

    if (PlayerComponent->GetTargetCharacter() && (!ClosestCharacter.IsValid() || PlayerComponent->GetTargetCharacter() != ClosestCharacter.Get()))
    {
        ClearTargetCharacter();
    }

    if (ClosestCharacter.IsValid() && !PlayerComponent->GetTargetCharacter())
    {
        SetTargetCharacter(ClosestCharacter);
    }
}

void UInworldPlayerTargetingComponent::SetTargetCharacter(TWeakObjectPtr<UInworldCharacterComponent> Character)
{
    if (ChangeTargetCharacterTimer.CheckPeriod(GetWorld()))
    {
        PlayerComponent->SetTargetInworldCharacter(Character.Get());
    }
}

void UInworldPlayerTargetingComponent::ClearTargetCharacter()
{
    if (PlayerComponent->GetTargetCharacter() && ChangeTargetCharacterTimer.CheckPeriod(GetWorld()))
    {
        PlayerComponent->ClearTargetInworldCharacter();
    }
}

