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
}

void UInworldPlayerComponent::InitializeComponent()
{
    Super::InitializeComponent();

    InworldPlayer = NewObject<UInworldPlayer>(this, "InworldPlayer");
}

void UInworldPlayerComponent::UninitializeComponent()
{
    Super::UninitializeComponent();

    InworldPlayer = nullptr;
}

void UInworldPlayerComponent::BeginPlay()
{
	Super::BeginPlay();

    InworldPlayer = NewObject<UInworldPlayer>(this);

    InworldSession = IInworldSessionOwnerInterface::Execute_GetInworldSession(GetWorld()->GetSubsystem<UInworldApiSubsystem>());

	SetIsReplicated(true);
}

void UInworldPlayerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    InworldPlayer = nullptr;

    Super::EndPlay(EndPlayReason);
}

void UInworldPlayerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
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
    for (UInworldCharacter* Character : InworldPlayer->GetTargetInworldCharacters())
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
    if (InworldPlayer->GetTargetInworldCharacters().Num() > 1)
    {
        InworldSession->BroadcastTrigger(InworldPlayer->GetTargetInworldCharacters(), "inworld.conversation.next_turn", {});
    }
}

void UInworldPlayerComponent::SetTargetInworldCharacter(UInworldCharacterComponent* Character)
{
    InworldPlayer->AddTargetInworldCharacter(Character->GetInworldCharacter());
}

void UInworldPlayerComponent::ClearTargetInworldCharacter(UInworldCharacterComponent* Character)
{
    InworldPlayer->RemoveTargetInworldCharacter(Character->GetInworldCharacter());
}

void UInworldPlayerComponent::ClearAllTargetInworldCharacters()
{
    InworldPlayer->ClearAllTargetInworldCharacters();
}

void UInworldPlayerComponent::SendTextMessageToTarget_Implementation(const FString& Message)
{
    if (!Message.IsEmpty())
    {
        InworldSession->BroadcastTextMessage(InworldPlayer->GetTargetInworldCharacters(), Message);
    }
}

void UInworldPlayerComponent::SendTriggerToTarget(const FString& Name, const TMap<FString, FString>& Params)
{
    InworldSession->BroadcastTrigger(InworldPlayer->GetTargetInworldCharacters(), Name, Params);
}

void UInworldPlayerComponent::StartAudioSessionWithTarget()
{
    InworldSession->BroadcastAudioSessionStart(InworldPlayer->GetTargetInworldCharacters());
}

void UInworldPlayerComponent::StopAudioSessionWithTarget()
{
    InworldSession->BroadcastAudioSessionStop(InworldPlayer->GetTargetInworldCharacters());
}

void UInworldPlayerComponent::SendAudioMessageToTarget(const TArray<uint8>& InputData, const TArray<uint8>& OutputData)
{
    InworldSession->BroadcastSoundMessage(InworldPlayer->GetTargetInworldCharacters(), InputData, OutputData);
}
