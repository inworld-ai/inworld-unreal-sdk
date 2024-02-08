/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldPlayerTargetingComponent.h"
#include "InworldPlayerComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"


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

    UpdateTargetCharacters();
}

void UInworldPlayerTargetingComponent::UpdateTargetCharacters()
{
    auto* Player = PlayerComponent.Get();

    // prevent starting another audio session in multiplayer
    const auto* CurrentAudioSessionOwner = InworldSubsystem->GetAudioSessionOwner();
    if (CurrentAudioSessionOwner && CurrentAudioSessionOwner != Player->GetOwner())
    {
        return;
    }
    
    // clear all targets if just switched from multiple targeting
    if (!bMultipleTargets && TargetCharacters.Num() > 1)
    {
        for (auto* Character : TargetCharacters)
        {
            Player->ClearTargetInworldCharacter(Character);
        }
        TargetCharacters.Empty();
    }

    // clear all targets out of range
    const float MinDistSq = InteractionDistance * InteractionDistance;
    const FVector Location = GetOwner()->GetActorLocation();
    for (int32 i = 0; i < TargetCharacters.Num(); i++)
    {
        auto* Character = TargetCharacters[i];
        const FVector CharacterLocation = Character->GetComponentOwner()->GetActorLocation();
        const float DistSq = FVector::DistSquared(Location, CharacterLocation);
        if (DistSq > MinDistSq)
        {
            Player->ClearTargetInworldCharacter(Character);
            TargetCharacters.RemoveAt(i);
            i--;
        }
    }

    const auto& CharacterComponents = InworldSubsystem->GetCharacterComponents();
    UInworldCharacterComponent* BestTarget = nullptr;
    float BestTargetDot = -1.f;
    for (auto& Char : CharacterComponents)
    {
        auto* Character = static_cast<UInworldCharacterComponent*>(Char);
        if (!Character || Character->GetAgentId().IsEmpty())
        {
            continue;
        }
        
        const FVector CharacterLocation = Character->GetComponentOwner()->GetActorLocation();
        const float DistSq = FVector::DistSquared(Location, CharacterLocation);
        if (DistSq > MinDistSq)
        {
            continue;
        }
        
        const auto* CurrentTargetPlayer = Character->GetTargetPlayer();
        if (CurrentTargetPlayer && CurrentTargetPlayer != Player)
        {
            continue;
        }

        // if multiple targets enabled add all characters in range
        if (bMultipleTargets)
        {
            int32 Idx;
            if (TargetCharacters.Find(Character, Idx))
            {
                continue;
            }

            TargetCharacters.Add(Character);
            PlayerComponent->SetTargetInworldCharacter(Character);
            continue;
        }

        // if multiple targets disabled target one character in range that we're looking at
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
        if (Dot < BestTargetDot)
        {
            continue;
        }

        BestTarget = Character;
        BestTargetDot = Dot;
    }

    if (bMultipleTargets)
    {
        return;
    }

    auto* CurTarget = TargetCharacters.Num() != 0 ? TargetCharacters[0] : nullptr;
    if (CurTarget != BestTarget)
    {
        if (CurTarget)
        {
			PlayerComponent->ClearTargetInworldCharacter(CurTarget);
			TargetCharacters.Empty();
        }

        if (BestTarget)
        {
			TargetCharacters.Add(BestTarget);
			PlayerComponent->SetTargetInworldCharacter(BestTarget);
        }
    }
}

