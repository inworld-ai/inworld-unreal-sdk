/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldPlayerComponent.h"
#include "NDK/Proto/ProtoDisableWarning.h"
#include "InworldApi.h"
#include "InworldCharacterComponent.h"

void UInworldPlayerComponent::BeginPlay()
{
    Super::BeginPlay();

    InworldSubsystem = GetWorld()->GetSubsystem<UInworldApiSubsystem>();
}

void UInworldPlayerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
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

void UInworldPlayerComponent::SendTextMessageToTarget(const FString& Message)
{
    if (!TargetCharacterAgentId.IsEmpty())
    {
        InworldSubsystem->SendTextMessage(TargetCharacterAgentId, Message);
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

void UInworldPlayerComponent::SendAudioDataMessageToTarget(const std::string& Data)
{
    if (!TargetCharacterAgentId.IsEmpty())
    {
        InworldSubsystem->SendAudioDataMessage(TargetCharacterAgentId, Data);
    }
}

void UInworldPlayerComponent::SendAudioDataMessageWithAECToTarget(const std::vector<int16_t>& InputData, const std::vector<int16_t>& OutputData)
{
    if (!TargetCharacterAgentId.IsEmpty())
    {
        InworldSubsystem->SendAudioDataMessageWithAEC(TargetCharacterAgentId, InputData, OutputData);
    }
}
