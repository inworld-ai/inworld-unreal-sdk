/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldPlayerMultiTargetingComponent.h"
#include "InworldPlayerComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"


UInworldPlayerMultiTargetingComponent::UInworldPlayerMultiTargetingComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UInworldPlayerMultiTargetingComponent::BeginPlay()
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

void UInworldPlayerMultiTargetingComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdateTargetCharacters();
}

void UInworldPlayerMultiTargetingComponent::UpdateTargetCharacters()
{
    const float MinDistSq = InteractionDistance * InteractionDistance;
    const FVector Location = GetOwner()->GetActorLocation();
    for (int32 i = 0; i < TargetCharacters.Num(); i++)
    {
        auto* Character = TargetCharacters[i];
        const FVector CharacterLocation = Character->GetComponentOwner()->GetActorLocation();
        const float DistSq = FVector::DistSquared(Location, CharacterLocation);
        if (DistSq > MinDistSq)
        {
            PlayerComponent->ClearTargetInworldCharacter(Character);
            TargetCharacters.RemoveAt(i);
            i--;
        }
    }

    const auto& CharacterComponents = InworldSubsystem->GetCharacterComponents();
    for (auto& Char : CharacterComponents)
    {
        auto* Character = static_cast<UInworldCharacterComponent*>(Char);
        if (!Character || Character->GetAgentId().IsEmpty())
        {
            continue;
        }

        if (Character->GetTargetPlayer())
        {
            continue;
        }
        
        const FVector CharacterLocation = Character->GetComponentOwner()->GetActorLocation();
        const float DistSq = FVector::DistSquared(Location, CharacterLocation);
        if (DistSq > MinDistSq)
        {
            continue;
        }

        int32 Idx;
        if (TargetCharacters.Find(Character, Idx))
        {
            continue;
        }

        TargetCharacters.Add(Character);
        PlayerComponent->SetTargetInworldCharacter(Character);
    }
}

