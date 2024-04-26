/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */


#include "InworldSessionActor.h"
#include "InworldSession.h"
#include "InworldApi.h"
#include <Net/UnrealNetwork.h>

AInworldSessionActor::AInworldSessionActor()
{
 	PrimaryActorTick.bCanEverTick = true;
	SetReplicates(true);
	bAlwaysRelevant = true;
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
	bReplicateUsingRegisteredSubObjectList = true;
#endif
}

void AInworldSessionActor::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	if (GetLocalRole() == ROLE_Authority)
	{
		UWorld* World = GetWorld();
		if (World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE))
		{
			InworldSession = NewObject<UInworldSession>(this);
			InworldSession->Init();
			OnRep_InworldSession();
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
			AddReplicatedSubObject(InworldSession);
#endif
		}
	}
}

void AInworldSessionActor::BeginPlay()
{
	Super::BeginPlay();
	OnRep_InworldSession();
}

void AInworldSessionActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (GetLocalRole() == ROLE_Authority)
	{
		InworldSession->Destroy();
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
}

void AInworldSessionActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AInworldSessionActor, InworldSession);
}

bool AInworldSessionActor::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
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

void AInworldSessionActor::OnRep_InworldSession()
{
	GetWorld()->GetSubsystem<UInworldApiSubsystem>()->SetInworldSession(InworldSession);
}

