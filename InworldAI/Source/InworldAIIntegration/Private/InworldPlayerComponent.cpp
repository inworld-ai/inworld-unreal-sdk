/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
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

	DOREPLIFETIME(UInworldPlayerComponent, TargetCharacterAgentId);
}

Inworld::ICharacterComponent* UInworldPlayerComponent::GetTargetCharacter()
{
    if (InworldSubsystem.IsValid() && !TargetCharacterAgentId.IsEmpty())
    {
        return InworldSubsystem->GetCharacterComponentByAgentId(TargetCharacterAgentId);
    }

    return nullptr;
}

void UInworldPlayerComponent::SetTargetInworldCharacter(UInworldCharacterComponent* Character)
{
    if (!ensureMsgf(Character && !Character->GetAgentId().IsEmpty(), TEXT("UInworldPlayerComponent::SetTargetCharacter: the Character must have valid AgentId")))
    {
        return;
    }

    ClearTargetInworldCharacter();

    if (Character->StartPlayerInteraction(this))
    {
        CharacterTargetUnpossessedHandle = Character->OnUnpossessed.AddUObject(this, &UInworldPlayerComponent::ClearTargetInworldCharacter);
        TargetCharacterAgentId = Character->GetAgentId();
        OnTargetSet.Broadcast(Character);
    }
}

void UInworldPlayerComponent::ClearTargetInworldCharacter()
{
    UInworldCharacterComponent* TargetCharacter = GetTargetInworldCharacter();
    if (TargetCharacter && TargetCharacter->StopPlayerInteraction(this))
    {
        TargetCharacter->OnUnpossessed.Remove(CharacterTargetUnpossessedHandle);
        OnTargetClear.Broadcast(TargetCharacter);
		TargetCharacterAgentId = FString();
    }
}

void UInworldPlayerComponent::SendTextMessageToTarget_Implementation(const FString& Message)
{
    if (!Message.IsEmpty() && !TargetCharacterAgentId.IsEmpty())
    {
        InworldSubsystem->SendTextMessage(TargetCharacterAgentId, Message);
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
    if (!TargetCharacterAgentId.IsEmpty())
    {
        InworldSubsystem->SendTrigger(TargetCharacterAgentId, Name, Params);
    }
}

void UInworldPlayerComponent::StartAudioSessionWithTarget()
{
    if (!TargetCharacterAgentId.IsEmpty())
    {
        InworldSubsystem->StartAudioSession(TargetCharacterAgentId);
    }
}

void UInworldPlayerComponent::StopAudioSessionWithTarget()
{
    if (!TargetCharacterAgentId.IsEmpty())
	{
		InworldSubsystem->StopAudioSession(TargetCharacterAgentId);
	}
}

void UInworldPlayerComponent::SendAudioMessageToTarget(USoundWave* SoundWave)
{
    if (!TargetCharacterAgentId.IsEmpty())
    {
        InworldSubsystem->SendAudioMessage(TargetCharacterAgentId, SoundWave);
    }
}

void UInworldPlayerComponent::SendAudioDataMessageToTarget(const TArray<uint8>& Data)
{
    if (!TargetCharacterAgentId.IsEmpty())
    {
        InworldSubsystem->SendAudioDataMessage(TargetCharacterAgentId, Data);
    }
}

void UInworldPlayerComponent::SendAudioDataMessageWithAECToTarget(const TArray<uint8>& InputData, const TArray<uint8>& OutputData)
{
    if (!TargetCharacterAgentId.IsEmpty())
    {
        InworldSubsystem->SendAudioDataMessageWithAEC(TargetCharacterAgentId, InputData, OutputData);
    }
}

void UInworldPlayerComponent::OnRep_TargetCharacterAgentId(FString OldAgentId)
{
    if (!ensure(InworldSubsystem.IsValid()))
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
	}
}
