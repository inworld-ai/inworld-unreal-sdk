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

void UInworldPlayerComponent::BeginPlay()
{
	Super::BeginPlay();

	SetIsReplicated(true);

    InworldSubsystem = GetWorld()->GetSubsystem<UInworldApiSubsystem>();
}

void UInworldPlayerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
}

void UInworldPlayerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInworldPlayerComponent, Targets);
}

TArray<UInworldCharacterComponent*> UInworldPlayerComponent::GetTargetInworldCharacters()
{
    if (!InworldSubsystem.IsValid())
    {
        return {};
    }

    TArray<UInworldCharacterComponent*> Result;
    for (auto& Target : Targets)
    {
        if (!Target.AgentId.IsEmpty())
        {
            Result.Add(static_cast<UInworldCharacterComponent*>(InworldSubsystem->GetCharacterComponentByAgentId(Target.AgentId)));
        }
    }
    
    return Result;
}

TArray<FString> UInworldPlayerComponent::GetTargetAgentIds()
{
    TArray<FString> AgentIds;
    for (auto& Target : Targets)
    {
        AgentIds.Add(Target.AgentId);
    }
    return AgentIds;
}

void UInworldPlayerComponent::ContinueMultiAgentConversation()
{
    InworldSubsystem->SendTriggerToConversation(ConversationId, "inworld.conversation.next_turn", {});
}

Inworld::ICharacterComponent* UInworldPlayerComponent::GetTargetCharacter()
{
    if (Targets.Num() == 0)
    {
        return nullptr;
    }

	if (InworldSubsystem.IsValid() && !Targets[0].AgentId.IsEmpty())
	{
		return InworldSubsystem->GetCharacterComponentByAgentId(Targets[0].AgentId);
	}

	return nullptr;
}

void UInworldPlayerComponent::SetTargetInworldCharacter(UInworldCharacterComponent* Character)
{
    if (!ensureMsgf(Character && !Character->GetAgentId().IsEmpty(), TEXT("UInworldPlayerComponent::SetTargetCharacter: the Character must have valid AgentId")))
    {
        return;
    }

    if (!Character->StartPlayerInteraction(this))
    {
        return;
    }

    if (Targets.FindByPredicate([Character](const auto& T) { return Character->GetAgentId() == T.AgentId; }) != nullptr)
    {
        return;
    }

    FInworldPlayerTargetCharacter Target;
    Target.UnpossessedHandle = Character->OnUnpossessed.AddLambda([this, Character]() { ClearTargetInworldCharacter(Character); });
    Target.AgentId = Character->GetAgentId();
    Targets.Add(Target);
    OnTargetSet.Broadcast(Character);
    ConversationId = InworldSubsystem->UpdateConversation(ConversationId, false, GetTargetAgentIds());
}

void UInworldPlayerComponent::ClearTargetInworldCharacter(UInworldCharacterComponent* Character)
{
    if (!Character)
    {
        return;
    }

    auto* Target = Targets.FindByPredicate([Character](const auto& T) { return Character->GetAgentId() == T.AgentId; });
    if (!Target)
    {
        return;
    }

    if (Character && Character->StopPlayerInteraction(this))
    {
        Character->OnUnpossessed.Remove(Target->UnpossessedHandle);
        OnTargetClear.Broadcast(Character);
    }

    Targets.RemoveAt(Target - &Targets[0]);
    ConversationId = InworldSubsystem->UpdateConversation(ConversationId, false, GetTargetAgentIds());
}

void UInworldPlayerComponent::ClearAllTargetInworldCharacters()
{
    const int32 NumTargets = Targets.Num();
    for (int32 i = 0; i < NumTargets; i++)
    {
        UInworldCharacterComponent* TargetCharacter = static_cast<UInworldCharacterComponent*>(InworldSubsystem->GetCharacterComponentByAgentId(Targets[0].AgentId));
        ClearTargetInworldCharacter(TargetCharacter);
    }
}

void UInworldPlayerComponent::SendTextMessageToTarget_Implementation(const FString& Message)
{
    InworldSubsystem->SendTextMessageToConversation(ConversationId, Message);
}

void UInworldPlayerComponent::SendTextMessage_Implementation(const FString& Message, const FString& AgentId)
{
    InworldSubsystem->SendTextMessage(AgentId, Message);
}

void UInworldPlayerComponent::SendTriggerToTarget(const FString& Name, const TMap<FString, FString>& Params)
{
    InworldSubsystem->SendTriggerToConversation(ConversationId, Name, Params);
}

void UInworldPlayerComponent::StartAudioSessionWithTarget()
{
    InworldSubsystem->StartAudioSessionInConversation(ConversationId, GetOwner());
}

void UInworldPlayerComponent::StopAudioSessionWithTarget()
{
    for (auto& Target : Targets)
    {
		InworldSubsystem->StopAudioSession(Target.AgentId);
	}
}

void UInworldPlayerComponent::SendAudioMessageToTarget(USoundWave* SoundWave)
{
    InworldSubsystem->SendAudioMessageToConversation(ConversationId, SoundWave);
}

void UInworldPlayerComponent::SendAudioDataMessageToTarget(const TArray<uint8>& Data)
{
    InworldSubsystem->SendAudioDataMessageToConversation(ConversationId, Data);
}

void UInworldPlayerComponent::SendAudioDataMessageWithAECToTarget(const TArray<uint8>& InputData, const TArray<uint8>& OutputData)
{
    InworldSubsystem->SendAudioDataMessageWithAECToConversation(ConversationId, InputData, OutputData);
}

void UInworldPlayerComponent::OnRep_Targets(const TArray<FInworldPlayerTargetCharacter>& OldTargets)
{
    if (!ensure(InworldSubsystem.IsValid()))
    {
        return;
    }

    for (auto& Target : OldTargets)
    {
        if (Targets.FindByPredicate([&Target](const auto& T) { return T.AgentId == Target.AgentId; }) == nullptr)
        {
            auto* Component = static_cast<UInworldCharacterComponent*>(InworldSubsystem->GetCharacterComponentByAgentId(Target.AgentId));
            OnTargetClear.Broadcast(Component);
        }
    }

    for (auto& Target : Targets)
    {
        if (OldTargets.FindByPredicate([&Target](const auto& T) { return T.AgentId == Target.AgentId; }) == nullptr)
        {
            auto* Component = static_cast<UInworldCharacterComponent*>(InworldSubsystem->GetCharacterComponentByAgentId(Target.AgentId));
            OnTargetSet.Broadcast(Component);
        }
    }
}
