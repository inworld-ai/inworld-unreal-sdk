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
        InworldPlayer = NewObject<UInworldPlayer>(this);
    }
    /*InworldPlayer->OnTargetCharacterAdded().AddLambda(
        [this](UInworldCharacter* Character) -> void
        {
            //TargetCharacters.Add(Character->GetAgentInfo().AgentId);
        }
    );*/
}

void UInworldPlayerComponent::UninitializeComponent()
{
    Super::UninitializeComponent();

    InworldPlayer = nullptr;
}

void UInworldPlayerComponent::BeginPlay()
{
	Super::BeginPlay();
    if (GetNetMode() != NM_Client)
    {
        InworldSession = IInworldSessionOwnerInterface::Execute_GetInworldSession(GetWorld()->GetSubsystem<UInworldApiSubsystem>());
    }
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
        UInworldCharacterComponent* InworldCharacterComponent = Cast<UInworldCharacterComponent>(Character->GetInworldCharacterOwner().GetObject());
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
        InworldSession->BroadcastTrigger(InworldPlayer->GetTargetCharacters(), "inworld.conversation.next_turn", {});
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

void UInworldPlayerComponent::SendTextMessageToTarget_Implementation(const FString& Message)
{
    if (!Message.IsEmpty())
    {
        InworldSession->BroadcastTextMessage(InworldPlayer->GetTargetCharacters(), Message);
    }
}

void UInworldPlayerComponent::SendTriggerToTarget(const FString& Name, const TMap<FString, FString>& Params)
{
    InworldSession->BroadcastTrigger(InworldPlayer->GetTargetCharacters(), Name, Params);
}

void UInworldPlayerComponent::StartAudioSessionWithTarget()
{
    InworldSession->BroadcastAudioSessionStart(InworldPlayer->GetTargetCharacters());
}

void UInworldPlayerComponent::StopAudioSessionWithTarget()
{
    InworldSession->BroadcastAudioSessionStop(InworldPlayer->GetTargetCharacters());
}

void UInworldPlayerComponent::SendAudioMessageToTarget(const TArray<uint8>& InputData, const TArray<uint8>& OutputData)
{
    InworldSession->BroadcastSoundMessage(InworldPlayer->GetTargetCharacters(), InputData, OutputData);
}
