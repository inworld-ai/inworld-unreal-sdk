/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldPlayerComponent.h"
#include "InworldApi.h"
#include "InworldCharacterComponent.h"

#include <Engine/World.h>
#include <Net/UnrealNetwork.h>

UInworldPlayerComponent::UInworldPlayerComponent()
    : Super()
{
    bWantsInitializeComponent = true;
    SetIsReplicatedByDefault(true);
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
    bReplicateUsingRegisteredSubObjectList = true;
#endif
}

void UInworldPlayerComponent::InitializeComponent()
{
    Super::InitializeComponent();

    if (GetOwnerRole() == ROLE_Authority)
    {
        UWorld* World = GetWorld();
        if (World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE))
        {
            InworldPlayer = NewObject<UInworldPlayer>(this);
            InworldSession = GetWorld()->GetSubsystem<UInworldApiSubsystem>()->GetInworldSession();
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
            AddReplicatedSubObject(InworldPlayer);
#endif
        }
    }
}

void UInworldPlayerComponent::UninitializeComponent()
{
    Super::UninitializeComponent();

    if (IsValid(InworldPlayer))
    {
#if ENGINE_MAJOR_VERSION == 5
        InworldPlayer->MarkAsGarbage();
#endif

#if ENGINE_MAJOR_VERSION == 4
        InworldPlayer->MarkPendingKill();
#endif
    }
    InworldPlayer = nullptr;
}

void UInworldPlayerComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UInworldPlayerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    InworldPlayer = nullptr;

    Super::EndPlay(EndPlayReason);
}

void UInworldPlayerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UInworldPlayerComponent, InworldPlayer);
    DOREPLIFETIME(UInworldPlayerComponent, InworldSession);
}

bool UInworldPlayerComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
    return Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
#else
    bool WroteSomething = true;

    if (IsValid(InworldPlayer))
    {
        WroteSomething |= Channel->ReplicateSubobject(InworldPlayer, *Bunch, *RepFlags);
    }

    return WroteSomething;
#endif
}

UInworldCharacterComponent* UInworldPlayerComponent::GetTargetInworldCharacter()
{
    TArray<UInworldCharacterComponent*> TargetCharacters = GetTargetInworldCharacters();
    if (TargetCharacters.Num() > 0)
    {
        return TargetCharacters[0];
    }
    return nullptr;
}

TArray<UInworldCharacterComponent*> UInworldPlayerComponent::GetTargetInworldCharacters()
{
    TArray<UInworldCharacterComponent*> InworldCharacterComponents;
    for (UInworldCharacter* Character : InworldPlayer->GetTargetCharacters())
    {
        UInworldCharacterComponent* InworldCharacterComponent = Cast<UInworldCharacterComponent>(Character->GetOuter());
        if (InworldCharacterComponent)
        {
            InworldCharacterComponents.Add(InworldCharacterComponent);
        }
    }
    return InworldCharacterComponents;
}

void UInworldPlayerComponent::ContinueMultiAgentConversation()
{
    if (InworldPlayer->GetTargetCharacters().Num() > 1)
    {
        InworldPlayer->BroadcastTrigger("inworld.conversation.next_turn", {});
    }
}

void UInworldPlayerComponent::SetTargetInworldCharacter(UInworldCharacterComponent* Character)
{
    InworldPlayer->AddTargetCharacter(IInworldCharacterOwnerInterface::Execute_GetInworldCharacter(Character));
}

void UInworldPlayerComponent::ClearTargetInworldCharacter(UInworldCharacterComponent* Character)
{
    InworldPlayer->RemoveTargetCharacter(IInworldCharacterOwnerInterface::Execute_GetInworldCharacter(Character));
}

void UInworldPlayerComponent::ClearAllTargetInworldCharacters()
{
    InworldPlayer->ClearAllTargetCharacters();
}

void UInworldPlayerComponent::SendTextMessageToTarget(const FString& Message)
{
    if (!Message.IsEmpty())
    {
        InworldPlayer->BroadcastTextMessage(Message);
    }
}

void UInworldPlayerComponent::SendTriggerToTarget(const FString& Name, const TMap<FString, FString>& Params)
{
    InworldPlayer->BroadcastTrigger(Name, Params);
}

void UInworldPlayerComponent::StartAudioSessionWithTarget()
{
    InworldPlayer->BroadcastAudioSessionStart();
}

void UInworldPlayerComponent::StopAudioSessionWithTarget()
{
    InworldPlayer->BroadcastAudioSessionStop();
}

void UInworldPlayerComponent::SendAudioMessageToTarget(const TArray<uint8>& InputData, const TArray<uint8>& OutputData)
{
    InworldPlayer->BroadcastSoundMessage(InputData, OutputData);
}
