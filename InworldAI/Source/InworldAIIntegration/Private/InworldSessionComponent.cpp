/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */


#include "InworldSessionComponent.h"
#include "InworldApi.h"

#include <Engine/World.h>
#include <Net/UnrealNetwork.h>

UInworldSessionComponent::UInworldSessionComponent()
	: Super()
{
	SetIsReplicatedByDefault(true);
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
	bReplicateUsingRegisteredSubObjectList = true;
#endif
}

void UInworldSessionComponent::OnRegister()
{
	Super::OnRegister();

	UWorld* World = GetWorld();
	if (World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE) && World->GetNetMode() != NM_Client)
	{
		InworldSession = NewObject<UInworldSession>(this);
		InworldSession->Init();
		OnRep_InworldSession();
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
		AddReplicatedSubObject(InworldSession);
#endif
	}
}

void UInworldSessionComponent::OnUnregister()
{
	Super::OnUnregister();

	if (IsValid(InworldSession))
	{
#if ENGINE_MAJOR_VERSION == 5
		InworldSession->MarkAsGarbage();
#endif

#if ENGINE_MAJOR_VERSION == 4
		InworldSession->MarkPendingKill();
#endif
	}
	InworldSession = nullptr;
}


void UInworldSessionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInworldSessionComponent, InworldSession);
}

bool UInworldSessionComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
	return Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
#else
	bool WroteSomething = true;

	if (IsValid(InworldSession))
	{
		WroteSomething |= Channel->ReplicateSubobject(InworldSession, *Bunch, *RepFlags);
	}

	return WroteSomething;
#endif
}

void UInworldSessionComponent::OnRep_InworldSession()
{
	GetWorld()->GetSubsystem<UInworldApiSubsystem>()->SetInworldSession(InworldSession);
	OnSessionCreatedDelegateNative.Broadcast();
	OnSessionCreatedDelegate.Broadcast();
}
