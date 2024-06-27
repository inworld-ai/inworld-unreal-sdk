/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */


#include "InworldSessionComponent.h"
#include "InworldApi.h"
#include "InworldMacros.h"

#include "InworldAIIntegrationModule.h"

#include "Runtime/Launch/Resources/Version.h"
#include "TimerManager.h"
#include <Engine/World.h>
#include <Net/UnrealNetwork.h>

#define EMPTY_ARG_RETURN(Arg, Return) INWORLD_WARN_AND_RETURN_EMPTY(LogInworldAIIntegration, UInworldSessionComponent, Arg, Return)
#define NO_SESSION_RETURN(Return) EMPTY_ARG_RETURN(InworldSession, Return)
#define NO_CLIENT_RETURN(Return) NO_SESSION_RETURN(Return) EMPTY_ARG_RETURN(InworldSession->GetClient(), Return)

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

bool UInworldSessionComponent::GetIsLoaded() const
{
	NO_SESSION_RETURN(false)

	return InworldSession->IsLoaded();
}

void UInworldSessionComponent::StartSession()
{
	NO_CLIENT_RETURN(void())

	InworldSession->GetClient()->SetEnvironment(Environment);
	InworldSession->StartSession(SceneId, PlayerProfile, Auth, {}, {}, CapabilitySet);
}

void UInworldSessionComponent::StartSessionFromSave(const FInworldSave& Save)
{
	NO_CLIENT_RETURN(void())

	InworldSession->GetClient()->SetEnvironment(Environment);
	InworldSession->StartSession(SceneId, PlayerProfile, Auth, Save, {}, CapabilitySet);
}

void UInworldSessionComponent::StartSessionFromToken(const FInworldSessionToken& Token)
{
	NO_CLIENT_RETURN(void())

	InworldSession->GetClient()->SetEnvironment(Environment);
	InworldSession->StartSession(SceneId, PlayerProfile, Auth, {}, Token, CapabilitySet);
}

void UInworldSessionComponent::StopSession()
{
	NO_CLIENT_RETURN(void())

	InworldSession->StopSession();
}

void UInworldSessionComponent::PauseSession()
{
	NO_CLIENT_RETURN(void())

	InworldSession->PauseSession();
}

void UInworldSessionComponent::ResumeSession()
{
	NO_CLIENT_RETURN(void())

	InworldSession->ResumeSession();
}

FString UInworldSessionComponent::GetSessionId() const
{
	NO_SESSION_RETURN({})

	return InworldSession->GetSessionId();
}

void UInworldSessionComponent::SaveSession(FOnInworldSessionSavedCallback Callback)
{
	NO_SESSION_RETURN(void())

	InworldSession->SaveSession(Callback);
}

EInworldConnectionState UInworldSessionComponent::GetConnectionState() const
{
	NO_SESSION_RETURN(EInworldConnectionState::Idle)

	return InworldSession->GetConnectionState();
}

void UInworldSessionComponent::GetConnectionError(FString& OutErrorMessage, int32& OutErrorCode) const
{
	NO_SESSION_RETURN(void())

	return InworldSession->GetConnectionError(OutErrorMessage, OutErrorCode);
}

void UInworldSessionComponent::SetSceneId(const FString& InSceneId)
{
	SceneId = InSceneId;

	UWorld* World = GetWorld();
	if (World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE) && World->GetNetMode() != NM_Client)
	{
		NO_SESSION_RETURN(void())

		if (GetConnectionState() == EInworldConnectionState::Connected)
		{
			InworldSession->SendChangeSceneEvent(InSceneId);
		}
	}
}

void UInworldSessionComponent::SetPlayerProfile(const FInworldPlayerProfile& InPlayerProfile)
{
	PlayerProfile = InPlayerProfile;

	UWorld* World = GetWorld();
	if (World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE) && World->GetNetMode() != NM_Client)
	{
		NO_SESSION_RETURN(void())

		if (GetConnectionState() == EInworldConnectionState::Connected)
		{
			InworldSession->LoadPlayerProfile(InPlayerProfile);
		}
	}
}

void UInworldSessionComponent::SetCapabilities(const FInworldCapabilitySet& InCapabilitySet)
{
	CapabilitySet = InCapabilitySet;

	UWorld* World = GetWorld();
	if (World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE) && World->GetNetMode() != NM_Client)
	{
		NO_SESSION_RETURN(void())

		if (GetConnectionState() == EInworldConnectionState::Connected)
		{
			InworldSession->LoadCapabilities(InCapabilitySet);
		}
	}
}

void UInworldSessionComponent::OnRep_InworldSession()
{
	GetWorld()->GetSubsystem<UInworldApiSubsystem>()->SetInworldSession(InworldSession);
	OnSessionCreatedDelegateNative.Broadcast();
	OnSessionCreatedDelegate.Broadcast();

	const bool bIsLoaded = GetIsLoaded();
	OnSessionLoadedDelegateNative.Broadcast(bIsLoaded);
	OnSessionLoadedDelegate.Broadcast(bIsLoaded);

	InworldSession->OnLoaded().AddLambda(
		[this](bool bIsLoaded) -> void
		{
			OnSessionLoadedDelegateNative.Broadcast(bIsLoaded);
			OnSessionLoadedDelegate.Broadcast(bIsLoaded);
		}
	);

	const EInworldConnectionState ConnectionState = GetConnectionState();
	OnSessionConnectionStateChangedDelegateNative.Broadcast(ConnectionState);
	OnSessionConnectionStateChangedDelegate.Broadcast(ConnectionState);

	InworldSession->OnConnectionStateChanged().AddLambda(
		[this](EInworldConnectionState ConnectionState) -> void
		{
			OnSessionConnectionStateChangedDelegateNative.Broadcast(ConnectionState);
			OnSessionConnectionStateChangedDelegate.Broadcast(ConnectionState);

			UWorld* World = GetWorld();
			if (!World || World->bIsTearingDown)
			{
				return;
			}
			if (World->GetNetMode() != NM_Client)
			{
				if (ConnectionState == EInworldConnectionState::Connected)
				{
					CurrentRetryConnectionTime = 1.f;
				}

				if (ConnectionState == EInworldConnectionState::Disconnected)
				{
					if (CurrentRetryConnectionTime == 0.f)
					{
						ResumeSession();
					}
					else
					{
						World->GetTimerManager().SetTimer(RetryConnectionTimerHandle, this, &UInworldSessionComponent::ResumeSession, CurrentRetryConnectionTime);
					}
					CurrentRetryConnectionTime += FMath::Min(CurrentRetryConnectionTime + RetryConnectionIntervalTime, MaxRetryConnectionTime);
				}
			}
		}
	);
}

#undef EMPTY_ARG_RETURN
#undef NO_SESSION_RETURN
#undef NO_CLIENT_RETURN
