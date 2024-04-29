/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldPlayerTargetingComponent.h"
#include "InworldPlayer.h"
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
        TArray<UActorComponent*> PlayerOwnerComponents = GetOwner()->GetComponentsByInterface(UInworldPlayerOwnerInterface::StaticClass());
        if (ensureMsgf(!PlayerOwnerComponents.IsEmpty(), TEXT("The owner of the AudioCapture must contain an InworldPlayerOwner!")))
        {
            InworldPlayer = IInworldPlayerOwnerInterface::Execute_GetInworldPlayer(PlayerOwnerComponents[0]);
        }
    }
}

void UInworldPlayerTargetingComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (InworldPlayer.IsValid())
    {
        UpdateTargetCharacters();
    }
}

void UInworldPlayerTargetingComponent::UpdateTargetCharacters()
{
    // clear all targets if just switched from multiple targeting
    if (!bMultipleTargets && TargetCharacters.Num() > 1)
    {
        InworldPlayer->ClearAllTargetCharacters();
        TargetCharacters.Empty();
    }

    // clear all targets out of range
    const float MinDistSq = InteractionDistance * InteractionDistance;
    const FVector Location = GetOwner()->GetActorLocation();
    for (int32 i = 0; i < TargetCharacters.Num(); i++)
    {
        UInworldCharacter* Character = TargetCharacters[i].Get();
        AActor* OuterActor = Character->GetTypedOuter<AActor>();
        const FVector CharacterLocation = OuterActor != nullptr ? OuterActor->GetActorLocation() : FVector::ZeroVector;
        const float DistSq = FVector::DistSquared(Location, CharacterLocation);
        if (DistSq > MinDistSq)
        {
            InworldPlayer->RemoveTargetCharacter(Character);
            TargetCharacters.RemoveAt(i);
            i--;
        }
    }

    UInworldSession* InworldSession = IInworldPlayerOwnerInterface::Execute_GetInworldSession(InworldPlayer->GetOuter());
    if (!InworldSession)
    {
        return;
    }
    const TArray<UInworldCharacter*>& Characters = InworldSession->GetRegisteredCharacters();
    UInworldCharacter* BestTarget = nullptr;
    float BestTargetDot = -1.f;
    for (UInworldCharacter* Character : Characters)
    {
        if (!Character->IsPossessed())
        {
            continue;
        }

        UInworldPlayer* Player = Character->GetTargetPlayer();
        if (Player && Player != InworldPlayer)
        {
            continue;
        }

        AActor* OuterActor = Character->GetTypedOuter<AActor>();
        if (!OuterActor)
        {
            continue;
        }

        const FVector CharacterLocation = OuterActor->GetActorLocation();
        const float DistSq = FVector::DistSquared(Location, CharacterLocation);
        if (DistSq > MinDistSq)
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
            InworldPlayer->AddTargetCharacter(Character);
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

    UInworldCharacter* CurrentTarget = TargetCharacters.Num() != 0 ? TargetCharacters[0].Get() : nullptr;
    if (CurrentTarget != BestTarget)
    {
        if (CurrentTarget)
        {
			InworldPlayer->RemoveTargetCharacter(CurrentTarget);
			TargetCharacters.Empty();
        }

        if (BestTarget)
        {
			TargetCharacters.Add(BestTarget);
            InworldPlayer->AddTargetCharacter(BestTarget);
        }
    }
}

