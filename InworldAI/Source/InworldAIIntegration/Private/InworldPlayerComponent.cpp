/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldPlayerComponent.h"
#include "InworldApi.h"
#include "InworldMacros.h"
#include "InworldCharacterComponent.h"

#include "InworldAIIntegrationModule.h"

#include <Engine/World.h>
#include <Net/UnrealNetwork.h>
#include "Runtime/Launch/Resources/Version.h"

#define EMPTY_ARG_RETURN(Arg, Return) INWORLD_WARN_AND_RETURN_EMPTY(LogInworldAIIntegration, UInworldPlayerComponent, Arg, Return)
#define NO_PLAYER_RETURN(Return) EMPTY_ARG_RETURN(InworldPlayer, Return)

UInworldPlayerComponent::UInworldPlayerComponent()
    : Super()
{
    bWantsInitializeComponent = true;
    SetIsReplicatedByDefault(true);
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
    bReplicateUsingRegisteredSubObjectList = true;
#endif
}

void UInworldPlayerComponent::OnRegister()
{
    Super::OnRegister();

    UWorld* World = GetWorld();
    if (World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE))
    {
        InworldPlayer = NewObject<UInworldPlayer>(this);
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
        //AddReplicatedSubObject(InworldPlayer);
#endif
    }
}

void UInworldPlayerComponent::OnUnregister()
{
    Super::OnUnregister();

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

void UInworldPlayerComponent::InitializeComponent()
{
    Super::InitializeComponent();
    UWorld* World = GetWorld();
    if (World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE) && World->GetNetMode() != NM_DedicatedServer)
    {
        InworldPlayer->SetSession(World->GetSubsystem<UInworldApiSubsystem>()->GetInworldSession());
    }
}

void UInworldPlayerComponent::UninitializeComponent()
{
    Super::UninitializeComponent();
    UWorld* World = GetWorld();
    if (World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE) && World->GetNetMode() != NM_DedicatedServer)
    {
        InworldPlayer->SetSession(nullptr);
    }
}

void UInworldPlayerComponent::BeginPlay()
{
    Super::BeginPlay();

    if (GetOwnerRole() == ROLE_Authority)
    {
        InworldPlayer->SetConversationParticipation(bConversationParticipant);
    }
}

void UInworldPlayerComponent::SetConversationParticipation(bool bParticipating)
{
    if (InworldPlayer->IsConversationParticipant() != bParticipating)
    {
        InworldPlayer->SetConversationParticipation(bParticipating);
    }
    bConversationParticipant = InworldPlayer->IsConversationParticipant();
}

void UInworldPlayerComponent::ContinueConversation()
{
    NO_PLAYER_RETURN(void())

    InworldPlayer->SendTriggerToConversation(TEXT("inworld.conversation.next_turn"), {});
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

void UInworldPlayerComponent::AddTargetInworldCharacter(UInworldCharacterComponent* Character)
{
    NO_PLAYER_RETURN(void())
    EMPTY_ARG_RETURN(Character, void())

    InworldPlayer->AddTargetCharacter(IInworldCharacterOwnerInterface::Execute_GetInworldCharacter(Character));
}

void UInworldPlayerComponent::RemoveTargetInworldCharacter(UInworldCharacterComponent* Character)
{
    NO_PLAYER_RETURN(void())
    EMPTY_ARG_RETURN(Character, void())

    InworldPlayer->RemoveTargetCharacter(IInworldCharacterOwnerInterface::Execute_GetInworldCharacter(Character));
}

void UInworldPlayerComponent::ClearAllTargetInworldCharacters()
{
    NO_PLAYER_RETURN(void())

    InworldPlayer->ClearAllTargetCharacters();
}

void UInworldPlayerComponent::SendTextMessageToTarget(const FString& Message)
{
    NO_PLAYER_RETURN(void())
    EMPTY_ARG_RETURN(Message, void())

    InworldPlayer->SendTextMessageToConversation(Message);
}

void UInworldPlayerComponent::SendTriggerToTarget(const FString& Name, const TMap<FString, FString>& Params)
{
    NO_PLAYER_RETURN(void())
    EMPTY_ARG_RETURN(Name, void())

    InworldPlayer->SendTriggerToConversation(Name, Params);
}

void UInworldPlayerComponent::StartAudioSessionWithTarget()
{
    NO_PLAYER_RETURN(void())

    InworldPlayer->SendAudioSessionStartToConversation(FInworldAudioSessionOptions::Default());
}

void UInworldPlayerComponent::StopAudioSessionWithTarget()
{
    NO_PLAYER_RETURN(void())

    InworldPlayer->SendAudioSessionStopToConversation();
}

void UInworldPlayerComponent::SendAudioMessageToTarget(const TArray<uint8>& InputData, const TArray<uint8>& OutputData)
{
    NO_PLAYER_RETURN(void())
    EMPTY_ARG_RETURN(InputData, void())

    InworldPlayer->SendSoundMessageToConversation(InputData, OutputData);
}

#undef EMPTY_ARG_RETURN
#undef NO_PLAYER_RETURN
