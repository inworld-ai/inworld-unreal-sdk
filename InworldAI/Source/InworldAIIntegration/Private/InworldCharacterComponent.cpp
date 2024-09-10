/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldCharacterComponent.h"
#include "InworldApi.h"
#include "InworldMacros.h"
#include "InworldAIIntegrationModule.h"
#include "Engine/EngineBaseTypes.h"
#include "InworldPlayerComponent.h"
#include <Camera/CameraComponent.h>
#include <Net/UnrealNetwork.h>
#include <Engine/World.h>
#include <Engine/ActorChannel.h>
#include <GameFramework/GameStateBase.h>
#include <GameFramework/PlayerState.h>
#include "Runtime/Launch/Resources/Version.h"

#include "InworldCharacterAudioComponent.h"

#define EMPTY_ARG_RETURN(Arg, Return) INWORLD_WARN_AND_RETURN_EMPTY(LogInworldAIIntegration, UInworldCharacterComponent, Arg, Return)
#define NO_CHARACTER_RETURN(Return) EMPTY_ARG_RETURN(InworldCharacter, Return)

UInworldCharacterComponent::UInworldCharacterComponent()
	: Super()
	, MessageQueue(MakeShared<FCharacterMessageQueue>(this))
{
    PrimaryComponentTick.bCanEverTick = true;
    bWantsInitializeComponent = true;
	SetIsReplicatedByDefault(true);
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
	bReplicateUsingRegisteredSubObjectList = true;
#endif
}

void UInworldCharacterComponent::OnRegister()
{
	Super::OnRegister();

	UWorld* World = GetWorld();
	if (World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE) && World->GetNetMode() != NM_Client)
	{
		InworldCharacter = NewObject<UInworldCharacter>(this);
		OnRep_InworldCharacter();
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
		AddReplicatedSubObject(InworldCharacter);
#endif
	}
}

void UInworldCharacterComponent::OnUnregister()
{
	Super::OnUnregister();

	if (IsValid(InworldCharacter))
	{
#if ENGINE_MAJOR_VERSION == 5
		InworldCharacter->MarkAsGarbage();
#endif

#if ENGINE_MAJOR_VERSION == 4
		InworldCharacter->MarkPendingKill();
#endif
	}

	InworldCharacter = nullptr;
}

void UInworldCharacterComponent::InitializeComponent()
{
    Super::InitializeComponent();
	UWorld* World = GetWorld();
	if (World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE) && World->GetNetMode() != NM_Client)
	{
		InworldCharacter->SetSession(World->GetSubsystem<UInworldApiSubsystem>()->GetInworldSession());
	}

#if WITH_EDITOR
	if (GetWorld() == nullptr || !GetWorld()->IsPlayInEditor())
	{
		return;
	}
#endif

    if (GetNetMode() != NM_DedicatedServer)
    {
        for (auto& Type : PlaybackTypes)
        {
			if (ensureMsgf(Type != nullptr, TEXT("UInworldCharacterComponent contains null playback type!")))
			{
				auto* Pb = NewObject<UInworldCharacterPlayback>(this, Type);
				Pb->SetCharacterComponent(this);
				Playbacks.Add(Pb);
			}
        }
    }
}

void UInworldCharacterComponent::UninitializeComponent()
{
	Super::UninitializeComponent();

	UWorld* World = GetWorld();
	if (World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE) && World->GetNetMode() != NM_Client)
	{
		InworldCharacter->SetSession(nullptr);
	}

#if WITH_EDITOR
	if (GetWorld() == nullptr || !GetWorld()->IsPlayInEditor())
	{
		return;
	}
#endif

	if (GetNetMode() != NM_DedicatedServer)
	{
		Playbacks.Empty();
	}
}

void UInworldCharacterComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwnerRole() == ROLE_Authority)
	{
		InworldCharacter->SetBrainName(BrainName);
	}

    for (auto* Pb : Playbacks)
    {
        Pb->BeginPlay();
    }
}

void UInworldCharacterComponent::EndPlay(EEndPlayReason::Type Reason)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		InworldCharacter->SetBrainName({});
	}

    for (auto* Pb : Playbacks)
    {
        Pb->EndPlay();
        Pb->ClearCharacterComponent();
    }

    Super::EndPlay(Reason);
}

void UInworldCharacterComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	for (auto* Pb : Playbacks)
	{
		Pb->Tick(DeltaTime);
	}
}

void UInworldCharacterComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInworldCharacterComponent, InworldCharacter);
}

bool UInworldCharacterComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
	return Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
#else
	bool WroteSomething = true;

	if (IsValid(InworldCharacter))
	{
		WroteSomething |= Channel->ReplicateSubobject(InworldCharacter, *Bunch, *RepFlags);
	}

	return WroteSomething;
#endif
}

void UInworldCharacterComponent::SetBrainName(const FString& Name)
{
#if WITH_EDITOR
	UWorld* World = GetWorld();
	if (World == nullptr || !World->IsPlayInEditor())
	{
		BrainName = Name;
		return;
	}
#endif
	NO_CHARACTER_RETURN(void())
	
	BrainName = Name;

	InworldCharacter->SetBrainName(BrainName);
}

FString UInworldCharacterComponent::GetBrainName() const
{
	NO_CHARACTER_RETURN({})

	return InworldCharacter->GetAgentInfo().BrainName;
}

FString UInworldCharacterComponent::GetAgentId() const
{
	NO_CHARACTER_RETURN({})

	return InworldCharacter->GetAgentInfo().AgentId;
}

FString UInworldCharacterComponent::GetGivenName() const
{
	NO_CHARACTER_RETURN({})

	return InworldCharacter->GetAgentInfo().GivenName;
}

UInworldCharacterPlayback* UInworldCharacterComponent::GetPlayback(TSubclassOf<UInworldCharacterPlayback> Class) const
{
    for (auto* Pb : Playbacks)
    {
        if (Pb->GetClass()->IsChildOf(Class.Get()))
        {
            return Pb;
        }
    }
    return nullptr;
}

bool UInworldCharacterComponent::IsInteractingWithPlayer() const
{
	return InworldCharacter != nullptr && InworldCharacter->GetTargetPlayer() != nullptr;
}

void UInworldCharacterComponent::Interrupt()
{
	MessageQueue->TryToInterrupt({});
}

void UInworldCharacterComponent::Interrupt(const FString& InterruptingInteractionId)
{
	EMPTY_ARG_RETURN(InterruptingInteractionId, void())

	MessageQueue->TryToInterrupt(InterruptingInteractionId);
}

void UInworldCharacterComponent::SendTextMessage(const FString& Text) const
{
	NO_CHARACTER_RETURN(void())
	EMPTY_ARG_RETURN(Text, void())

	InworldCharacter->SendTextMessage(Text);
}

void UInworldCharacterComponent::SendTrigger(const FString& Name, const TMap<FString, FString>& Params) const
{
	NO_CHARACTER_RETURN(void())
	EMPTY_ARG_RETURN(Name, void())

	InworldCharacter->SendTrigger(Name, Params);
}

void UInworldCharacterComponent::SendNarrationEvent(const FString& Content)
{
	NO_CHARACTER_RETURN(void())
	EMPTY_ARG_RETURN(Content, void())

	InworldCharacter->SendNarrationEvent(Content);
}

void UInworldCharacterComponent::StartAudioSession(FInworldAudioSessionOptions SessionOptions)
{
	NO_CHARACTER_RETURN(void())

	InworldCharacter->SendAudioSessionStart(SessionOptions);
}

void UInworldCharacterComponent::StopAudioSession()
{
	NO_CHARACTER_RETURN(void())

	InworldCharacter->SendAudioSessionStop();
}

FVector UInworldCharacterComponent::GetTargetPlayerCameraLocation()
{
	if (InworldCharacter == nullptr || InworldCharacter->GetTargetPlayer() == nullptr)
	{
		return FVector::ZeroVector;
	}

	AActor* TargetPlayerActor = InworldCharacter->GetTargetPlayer()->GetTypedOuter<AActor>();
	UCameraComponent* CameraComponent = Cast<UCameraComponent>(TargetPlayerActor->GetComponentByClass(UCameraComponent::StaticClass()));
	if (!CameraComponent)
	{
		return GetOwner()->GetActorLocation();
	}

	return CameraComponent->K2_GetComponentLocation();
}

bool UInworldCharacterComponent::IsCustomGesture(const FString& CustomEventName) const
{
	return CustomEventName.Find("gesture") == 0;
}

void UInworldCharacterComponent::Multicast_VisitText_Implementation(const FInworldTextEvent& Event)
{
    if (GetNetMode() == NM_DedicatedServer)
    {
        return;
    }

	const auto& FromActor = Event.Routing.Source;
	if (FromActor.Type == EInworldActorType::AGENT)
	{
		if (Event.Final)
		{
			UE_LOG(LogInworldAIIntegration, Log, TEXT("From %s: %s"), *FromActor.Name, *Event.Text);
		}

		MessageQueue->AddOrUpdateMessage<FInworldTextEvent, FCharacterMessageUtterance>(Event);
	}
	else if (FromActor.Type == EInworldActorType::PLAYER)
	{
		if (Event.Final)
		{
			UE_LOG(LogInworldAIIntegration, Log, TEXT("To %s: %s"), *Event.Routing.Target.Name, *Event.Text);
		}

		// Don't add to queue, player talking is instant.
		FCharacterMessagePlayerTalk PlayerTalk;
		PlayerTalk << Event;

		OnPlayerTalk.Broadcast(PlayerTalk);

		TSharedPtr<FCharacterMessage> CurrentMessage = GetCurrentMessage();
		if (CurrentMessage.IsValid() && CurrentMessage->InteractionId != Event.PacketId.InteractionId)
		{
			Interrupt(Event.PacketId.InteractionId);
		}
	}
}

void UInworldCharacterComponent::Multicast_VisitVAD_Implementation(const FInworldVADEvent& Event)
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}
	
	if (Event.VoiceDetected)
	{
		MessageQueue->TryToPause();
	}
	else
	{
		MessageQueue->TryToResume();
	}
	OnVoiceDetection.Broadcast(Event.VoiceDetected);
}

void UInworldCharacterComponent::VisitAudioOnClient(const FInworldAudioDataEvent& Event)
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	MessageQueue->AddOrUpdateMessage<FInworldAudioDataEvent, FCharacterMessageUtterance>(Event);
}

void UInworldCharacterComponent::Multicast_VisitSilence_Implementation(const FInworldSilenceEvent& Event)
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	MessageQueue->AddOrUpdateMessage<FInworldSilenceEvent, FCharacterMessageSilence>(Event);
}

void UInworldCharacterComponent::Multicast_VisitControl_Implementation(const FInworldControlEvent& Event)
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	if (Event.Action == EInworldControlEventAction::INTERACTION_END)
	{
		MessageQueue->AddOrUpdateMessage<FInworldControlEvent, FCharacterMessageInteractionEnd>(Event);
	}
}

void UInworldCharacterComponent::Multicast_VisitCustom_Implementation(const FInworldCustomEvent& Event)
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	UE_LOG(LogInworldAIIntegration, Log, TEXT("CustomEvent arrived: %s - %s"), *Event.Name, *Event.PacketId.InteractionId);

	MessageQueue->AddOrUpdateMessage<FInworldCustomEvent, FCharacterMessageTrigger>(Event);
}

void UInworldCharacterComponent::Multicast_VisitRelation_Implementation(const FInworldRelationEvent& Event)
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	MessageQueue->AddOrUpdateMessage<FInworldRelationEvent, FCharacterMessageTrigger>(Event);
}

void UInworldCharacterComponent::Multicast_VisitEmotion_Implementation(const FInworldEmotionEvent& Event)
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	EmotionStrength = static_cast<EInworldCharacterEmotionStrength>(Event.Strength);

	if (Event.Behavior != EmotionalBehavior)
	{
		EmotionalBehavior = Event.Behavior;
		OnEmotionalBehaviorChanged.Broadcast(EmotionalBehavior, EmotionStrength);
	}
}

void UInworldCharacterComponent::OnInworldTextEvent(const FInworldTextEvent& Event)
{
    Multicast_VisitText(Event);
}

void UInworldCharacterComponent::OnInworldVADEvent(const FInworldVADEvent& Event)
{
	Multicast_VisitVAD(Event);
}

bool IsA2FEnabled(UInworldSession* InworldSession)
{
	EMPTY_ARG_RETURN(InworldSession, false)
	UInworldClient* InworldClient = InworldSession->GetClient();
	EMPTY_ARG_RETURN(InworldClient, false)
	return InworldClient->GetCapabilities().Audio2Face;
}

void UInworldCharacterComponent::OnInworldAudioEvent(const FInworldAudioDataEvent& Event)
{
	// TODO: Support Networked A2F
	if (IsA2FEnabled(InworldCharacter->GetSession()))
	{
		return;
	}

	if (GetNetMode() == NM_Standalone || GetNetMode() == NM_Client)
	{
		VisitAudioOnClient(Event);
		return;
	}

	if (GetNetMode() == NM_ListenServer)
	{
		VisitAudioOnClient(Event);
	}

	UInworldApiSubsystem* InworldSubsystem = GetWorld()->GetSubsystem<UInworldApiSubsystem>();
	if (ensure(InworldSubsystem))
	{
		TArray<FInworldAudioDataEvent> RepEvents;
		FInworldAudioDataEvent::ConvertToReplicatableEvents(Event, RepEvents);

		for (auto& E : RepEvents)
		{
			InworldSubsystem->ReplicateAudioEventFromServer(E);
		}
	}
}

void UInworldCharacterComponent::OnInworldA2FHeaderEvent(const FInworldA2FHeaderEvent& Event)
{
	// TODO: Support Networked A2F
	MessageQueue->AddOrUpdateMessage<FInworldA2FHeaderEvent, FCharacterMessageUtterance>(Event);
}

void UInworldCharacterComponent::OnInworldA2FContentEvent(const FInworldA2FContentEvent& Event)
{
	// TODO: Support Networked A2F
	MessageQueue->AddOrUpdateMessage<FInworldA2FContentEvent, FCharacterMessageUtterance>(Event);
}

void UInworldCharacterComponent::OnInworldSilenceEvent(const FInworldSilenceEvent& Event)
{
    Multicast_VisitSilence(Event);
}

void UInworldCharacterComponent::OnInworldControlEvent(const FInworldControlEvent& Event)
{
    Multicast_VisitControl(Event);
}

void UInworldCharacterComponent::OnInworldEmotionEvent(const FInworldEmotionEvent& Event)
{
    Multicast_VisitEmotion(Event);
}

void UInworldCharacterComponent::OnInworldCustomEvent(const FInworldCustomEvent& Event)
{
	Multicast_VisitCustom(Event);
}

void UInworldCharacterComponent::OnInworldRelationEvent(const FInworldRelationEvent& Event)
{
	Multicast_VisitRelation(Event);
}

void UInworldCharacterComponent::Handle(const FCharacterMessageUtterance& Message)
{
	OnUtterance.Broadcast(Message);
}

void UInworldCharacterComponent::Interrupt(const FCharacterMessageUtterance& Message)
{
	const FString& InteractionId = Message.InteractionId;
	if (!PendingCancelResponses.Contains(Message.InteractionId))
	{
		PendingCancelResponses.Add(InteractionId, {});
	}
	PendingCancelResponses[InteractionId].Add(Message.UtteranceId);

	OnUtteranceInterrupt.Broadcast(Message);
}

void UInworldCharacterComponent::Pause(const FCharacterMessageUtterance& Message)
{
	OnUtterancePause.Broadcast(Message);
}

void UInworldCharacterComponent::Resume(const FCharacterMessageUtterance& Message)
{
	OnUtteranceResume.Broadcast(Message);
}

void UInworldCharacterComponent::Handle(const FCharacterMessageSilence& Message)
{
	OnSilence.Broadcast(Message);
}

void UInworldCharacterComponent::Interrupt(const FCharacterMessageSilence& Message)
{
	OnSilenceInterrupt.Broadcast(Message);
}

void UInworldCharacterComponent::Handle(const FCharacterMessageTrigger& Message)
{
	OnTrigger.Broadcast(Message);
}

void UInworldCharacterComponent::Handle(const FCharacterMessageInteractionEnd& Message)
{
	const FString& InteractionId = Message.InteractionId;
	if (PendingCancelResponses.Contains(Message.InteractionId))
	{
		InworldCharacter->CancelResponse(InteractionId, PendingCancelResponses[InteractionId]);
	}
	OnInteractionEnd.Broadcast(Message);
}

void UInworldCharacterComponent::OnRep_InworldCharacter()
{
	if (InworldCharacter)
	{
		InworldCharacter->OnTargetPlayerChanged().AddLambda(
			[this]() -> void
			{
				OnPlayerInteractionStateChanged.Broadcast(InworldCharacter->GetTargetPlayer() != nullptr);
			}
		);
		OnPlayerInteractionStateChanged.Broadcast(InworldCharacter->GetTargetPlayer() != nullptr);

		InworldCharacter->OnInworldTextEvent().AddUObject(this, &UInworldCharacterComponent::OnInworldTextEvent);
		InworldCharacter->OnInworldVADEvent().AddUObject(this, &UInworldCharacterComponent::OnInworldVADEvent);
		InworldCharacter->OnInworldAudioEvent().AddUObject(this, &UInworldCharacterComponent::OnInworldAudioEvent);
		InworldCharacter->OnInworldA2FHeaderEvent().AddUObject(this, &UInworldCharacterComponent::OnInworldA2FHeaderEvent);
		InworldCharacter->OnInworldA2FContentEvent().AddUObject(this, &UInworldCharacterComponent::OnInworldA2FContentEvent);
		InworldCharacter->OnInworldSilenceEvent().AddUObject(this, &UInworldCharacterComponent::OnInworldSilenceEvent);
		InworldCharacter->OnInworldControlEvent().AddUObject(this, &UInworldCharacterComponent::OnInworldControlEvent);
		InworldCharacter->OnInworldEmotionEvent().AddUObject(this, &UInworldCharacterComponent::OnInworldEmotionEvent);
		InworldCharacter->OnInworldCustomEvent().AddUObject(this, &UInworldCharacterComponent::OnInworldCustomEvent);
	}
}

#undef EMPTY_ARG_RETURN
#undef NO_CHARACTER_RETURN
