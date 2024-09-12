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
		InworldSession->Destroy();
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

void UInworldSessionComponent::StartSession(const FString& SceneId, const FInworldSave& Save, const FInworldSessionToken& Token)
{
	NO_CLIENT_RETURN(void())

	InworldSession->GetClient()->SetEnvironment(Environment);
	InworldSession->StartSession(PlayerProfile, Auth, SceneId, Save, Token, CapabilitySet, Metadata);
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

void UInworldSessionComponent::SendInteractionFeedback(const FString& InteractionId, bool bIsLike, const FString& Message)
{
	NO_SESSION_RETURN(void())
	EMPTY_ARG_RETURN(InteractionId, void())

	InworldSession->SendInteractionFeedback(InteractionId, bIsLike, Message);
}

EInworldConnectionState UInworldSessionComponent::GetConnectionState() const
{
	NO_SESSION_RETURN(EInworldConnectionState::Idle)

	return InworldSession->GetConnectionState();
}

void UInworldSessionComponent::GetConnectionError(FString& OutErrorMessage, int32& OutErrorCode, FInworldConnectionErrorDetails& OutErrorDetails) const
{
	NO_SESSION_RETURN(void())

	return InworldSession->GetConnectionError(OutErrorMessage, OutErrorCode, OutErrorDetails);
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
					World->GetTimerManager().ClearTimer(RetryConnectionTimerHandle);
				}

				if (ConnectionState == EInworldConnectionState::Disconnected)
				{
					FString OutErrorMessage;
					int32 OutErrorCode;
					FInworldConnectionErrorDetails OutErrorDetails;
					GetConnectionError(OutErrorMessage, OutErrorCode, OutErrorDetails);
					if (OutErrorDetails.ReconnectionType == EInworldReconnectionType::IMMEDIATE)
					{
						UE_LOG(LogInworldAIIntegration, Log, TEXT("Attempting reconnection immediately: (%s, Code: %d)"), *OutErrorMessage, OutErrorCode);
						ResumeSession();
					}
					else if (OutErrorDetails.ReconnectionType == EInworldReconnectionType::TIMEOUT)
					{
						UE_LOG(LogInworldAIIntegration, Log, TEXT("Attempting reconnection after timeout: (%s, Code: %d)"), *OutErrorMessage, OutErrorCode);
						World->GetTimerManager().SetTimer(RetryConnectionTimerHandle, this, &UInworldSessionComponent::ResumeSession, OutErrorDetails.ReconnectTime);
					}
					else
					{
						UE_LOG(LogInworldAIIntegration, Warning, TEXT("Will not reattempt Reconnection for: (%s, Code: %d)"), *OutErrorMessage, OutErrorCode);
					}
				}
			}
		}
	);
}

#undef EMPTY_ARG_RETURN
#undef NO_SESSION_RETURN
#undef NO_CLIENT_RETURN
