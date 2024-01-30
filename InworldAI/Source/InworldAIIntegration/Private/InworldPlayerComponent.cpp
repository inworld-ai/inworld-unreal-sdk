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

    //TODO: support multi agent
	//DOREPLIFETIME(UInworldPlayerComponent, TargetCharacterAgentId);
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
    if (Targets.Num() > 1)
    {
        InworldSubsystem->SendTriggerMult(GetTargetAgentIds(), "inworld.conversation.next_turn", {});
    }
}

Inworld::ICharacterComponent* UInworldPlayerComponent::GetTargetCharacter()
{
    // TODO: deprecate?
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

    if (Targets.FindByPredicate([Character](const auto& T) { return Character->GetAgentId() == T.AgentId; }))
    {
        return;
    }

    FInworldPlayerTargetCharacter Target;
    Target.UnpossessedHandle = Character->OnUnpossessed.AddLambda([this, Character]() { ClearTargetInworldCharacter(Character); });
    Target.AgentId = Character->GetAgentId();
    Targets.Add(Target);
    OnTargetSet.Broadcast(Character);
}

void UInworldPlayerComponent::ClearTargetInworldCharacter(UInworldCharacterComponent* Character)
{
    if (!ensureMsgf(Character && !Character->GetAgentId().IsEmpty(), TEXT("UInworldPlayerComponent::ClearTargetInworldCharacter: the Character must have valid AgentId")))
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
}

void UInworldPlayerComponent::ClearAllTargetInworldCharacters()
{
    for (auto& Target : Targets)
    {
        UInworldCharacterComponent* TargetCharacter = static_cast<UInworldCharacterComponent*>(InworldSubsystem->GetCharacterComponentByAgentId(Target.AgentId));
        ClearTargetInworldCharacter(TargetCharacter);
    }
}

void UInworldPlayerComponent::SendTextMessageToTarget_Implementation(const FString& Message)
{
    if (!Message.IsEmpty())
    {
        for (auto& Target : Targets)
        {
            InworldSubsystem->SendTextMessage(Target.AgentId, Message);
        }
    }
}

void UInworldPlayerComponent::SendTextMessage_Implementation(const FString& Message, const FString& AgentId)
{
	if (!Message.IsEmpty() && !AgentId.IsEmpty())
	{
		InworldSubsystem->SendTextMessage(AgentId, Message);
	}
}

void UInworldPlayerComponent::SendTriggerToTarget(const FString& Name, const TMap<FString, FString>& Params)
{
    for (auto& Target : Targets)
    {
        InworldSubsystem->SendTrigger(Target.AgentId, Name, Params);
    }
}

void UInworldPlayerComponent::StartAudioSessionWithTarget()
{
    for (auto& Target : Targets)
    {
        InworldSubsystem->StartAudioSession(Target.AgentId);
    }
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
    for (auto& Target : Targets)
    {
        InworldSubsystem->SendAudioMessage(Target.AgentId, SoundWave);
    }
}

void UInworldPlayerComponent::SendAudioDataMessageToTarget(const TArray<uint8>& Data)
{
    for (auto& Target : Targets)
    {
        InworldSubsystem->SendAudioDataMessage(Target.AgentId, Data);
    }
}

void UInworldPlayerComponent::SendAudioDataMessageWithAECToTarget(const TArray<uint8>& InputData, const TArray<uint8>& OutputData)
{
    for (auto& Target : Targets)
    {
        InworldSubsystem->SendAudioDataMessageWithAEC(Target.AgentId, InputData, OutputData);
    }
}

void UInworldPlayerComponent::OnRep_Targets(const TArray<FInworldPlayerTargetCharacter>& OldTrgets)
{
    // TODO: support multiple targets
    /*if (!ensure(InworldSubsystem.IsValid()))
    {
        return;
    }

	const bool bHadTarget = !OldAgentId.IsEmpty();
	const bool bHasTarget = !TargetCharacterAgentId.IsEmpty();
	if (bHadTarget && !bHasTarget)
	{
        auto* Component = static_cast<UInworldCharacterComponent*>(InworldSubsystem->GetCharacterComponentByAgentId(OldAgentId));
        OnTargetClear.Broadcast(Component);
	}
	if (bHasTarget && OldAgentId != TargetCharacterAgentId)
	{
        auto* Component = static_cast<UInworldCharacterComponent*>(InworldSubsystem->GetCharacterComponentByAgentId(TargetCharacterAgentId));
		OnTargetSet.Broadcast(Component);
	}*/
}
