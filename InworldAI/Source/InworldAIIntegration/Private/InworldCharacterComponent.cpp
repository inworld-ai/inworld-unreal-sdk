/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldCharacterComponent.h"
#include "InworldApi.h"
#include "InworldAIIntegrationModule.h"
#include "Engine/EngineBaseTypes.h"
#include "InworldPlayerComponent.h"
#include <Camera/CameraComponent.h>
#include <Net/UnrealNetwork.h>
#include <Engine/World.h>
#include <GameFramework/GameStateBase.h>
#include <GameFramework/PlayerState.h>

UInworldCharacterComponent::UInworldCharacterComponent()
	: MessageQueue(MakeShared<FCharacterMessageQueue>(this))
{
    PrimaryComponentTick.bCanEverTick = true;
    bWantsInitializeComponent = true;
}

void UInworldCharacterComponent::InitializeComponent()
{
    Super::InitializeComponent();

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

	SetIsReplicated(true);

	InworldSubsystem = GetWorld()->GetSubsystem<UInworldApiSubsystem>();

	if (GetNetMode() != NM_Client)
	{
		Register();
	}

    for (auto* Pb : Playbacks)
    {
        Pb->BeginPlay();
    }
}

void UInworldCharacterComponent::EndPlay(EEndPlayReason::Type Reason)
{
    for (auto* Pb : Playbacks)
    {
        Pb->EndPlay();
		Pb->ClearCharacterComponent();
    }

	if (GetNetMode() == NM_Client)
	{
		if (InworldSubsystem.IsValid())
		{
			FString NewAgentId = FString();
			InworldSubsystem->UpdateCharacterComponentRegistrationOnClient(this, NewAgentId, AgentId);
		}
	}
	else
	{
		Unregister();
	}
	
	MessageQueue->Interrupt();

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

	DOREPLIFETIME(UInworldCharacterComponent, TargetPlayer);
	DOREPLIFETIME(UInworldCharacterComponent, AgentId);
}

void UInworldCharacterComponent::Possess(const FInworldAgentInfo& AgentInfo)
{
	AgentId = AgentInfo.AgentId;
	GivenName = AgentInfo.GivenName;
	OnPossessed.Broadcast();
}

void UInworldCharacterComponent::Unpossess()
{
	if (IsPossessing())
	{
		OnUnpossessed.Broadcast();
		AgentId = FString();
		GivenName = FString();
	}
}

void UInworldCharacterComponent::SetBrainName(const FString& Name)
{
#if WITH_EDITOR
	if (GetWorld() == nullptr || !GetWorld()->IsPlayInEditor())
	{
		BrainName = Name;
		return;
	}
#endif
	if (GetNetMode() == NM_Client)
	{
		return;
	}

	if (!ensure(InworldSubsystem.IsValid()))
	{
		return;
	}

	Unregister();
	BrainName = Name;
	Register();
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

void UInworldCharacterComponent::HandlePacket(TSharedPtr<FInworldPacket> Packet)
{
    if (ensure(Packet))
	{
		Packet->Accept(*this);
    }
}

Inworld::IPlayerComponent* UInworldCharacterComponent::GetTargetPlayer()
{
	return TargetPlayer;
}

bool UInworldCharacterComponent::StartPlayerInteraction(UInworldPlayerComponent* Player)
{
	if (TargetPlayer != nullptr)
	{
		return false;
	}

	TargetPlayer = Player;
	OnPlayerInteractionStateChanged.Broadcast(true);
	return true;
}

bool UInworldCharacterComponent::StopPlayerInteraction(UInworldPlayerComponent* Player)
{
	if (TargetPlayer != Player)
	{
		return false;
	}

	TargetPlayer = nullptr;
	OnPlayerInteractionStateChanged.Broadcast(false);
	return true;
}

bool UInworldCharacterComponent::IsInteractingWithPlayer() const
{
	return TargetPlayer != nullptr;
}

void UInworldCharacterComponent::Interrupt()
{
	if (AgentId.IsEmpty())
	{
		return;
	}

	TArray<FString> PendingInteractionIdsCopy = PendingInteractionIds;
	for (const FString& PendingInteractionId : PendingInteractionIdsCopy)
	{
		if (PendingInteractionId == CurrentInteractionId)
		{
			break;
		}
		PendingInteractionIds.Remove(PendingInteractionId);
		CanceledInteractionIds.Add(PendingInteractionId);
	}

	TMap<FString, TArray<FString>> InterruptedInteractions = MessageQueue->Interrupt();
	for (const auto& InterruptedInteraction : InterruptedInteractions)
	{
		InworldSubsystem->CancelResponse(AgentId, InterruptedInteraction.Key, InterruptedInteraction.Value);
	}
}

void UInworldCharacterComponent::SendTextMessage(const FString& Text) const
{
    if (ensure(!AgentId.IsEmpty()))
    {
        InworldSubsystem->SendTextMessage(AgentId, Text);
    }
}

void UInworldCharacterComponent::SendTrigger(const FString& Name, const TMap<FString, FString>& Params) const
{
    if (ensure(!AgentId.IsEmpty()))
    {
        InworldSubsystem->SendTrigger(AgentId, Name, Params);
    }
}

void UInworldCharacterComponent::SendAudioMessage(USoundWave* SoundWave) const
{
    if (ensure(!AgentId.IsEmpty()))
    {
        InworldSubsystem->SendAudioMessage(AgentId, SoundWave);
    }
}

void UInworldCharacterComponent::SendNarrationEvent(const FString& Content)
{
	if (ensure(!AgentId.IsEmpty()))
	{
		InworldSubsystem->SendNarrationEvent(AgentId, Content);
	}
}

void UInworldCharacterComponent::StartAudioSession(const AActor* Owner) const
{
    if (ensure(!AgentId.IsEmpty()))
    {
        InworldSubsystem->StartAudioSession(AgentId, Owner);
    }
}

void UInworldCharacterComponent::StopAudioSession() const
{
    if (ensure(!AgentId.IsEmpty()))
    {
        InworldSubsystem->StopAudioSession(AgentId);
    }
}

bool UInworldCharacterComponent::Register()
{
    if (BrainName.IsEmpty())
    {
        return false;
    }

	if (!ensure(InworldSubsystem.IsValid()))
	{
        return false;
	}

	if (InworldSubsystem->IsCharacterComponentRegistered(this))
	{
		return false;
	}

    InworldSubsystem->RegisterCharacterComponent(this);

    return true;
}

bool UInworldCharacterComponent::Unregister()
{
	if (!ensure(InworldSubsystem.IsValid()))
	{
		return false;
	}

	if (!InworldSubsystem->IsCharacterComponentRegistered(this))
	{
		return false;
	}

    InworldSubsystem->UnregisterCharacterComponent(this);

    return true;
}

FVector UInworldCharacterComponent::GetTargetPlayerCameraLocation()
{
	if (TargetPlayer == nullptr)
	{
		return FVector::ZeroVector;
	}

	UCameraComponent* CameraComponent = Cast<UCameraComponent>(TargetPlayer->GetOwner()->GetComponentByClass(UCameraComponent::StaticClass()));
	if (!CameraComponent)
	{
		return GetOwner()->GetActorLocation();
	}

	return CameraComponent->K2_GetComponentLocation();
}

void UInworldCharacterComponent::Multicast_VisitText_Implementation(const FInworldTextEvent& Event)
{
    if (GetNetMode() == NM_DedicatedServer)
    {
        return;
    }

	auto ProcessTarget = [this, Event](const FInworldActor& ToActor)
		{
			if (Event.Routing.Source.Type == EInworldActorType::PLAYER && ToActor.Type == EInworldActorType::AGENT && ToActor.Name == GetAgentId())
			{
				if (Event.Final)
				{
					UE_LOG(LogInworldAIIntegration, Log, TEXT("To %s: %s"), *ToActor.Name, *Event.Text);
				}

				// Don't add to queue, player talking is instant.
				FCharacterMessagePlayerTalk PlayerTalk;
				PlayerTalk.InteractionId = Event.PacketId.InteractionId;
				PlayerTalk.UtteranceId = Event.PacketId.UtteranceId;
				PlayerTalk.Text = Event.Text;
				PlayerTalk.bTextFinal = Event.Final;

				OnPlayerTalk.Broadcast(PlayerTalk);

				TSharedPtr<FCharacterMessage> CurrentMessage = GetCurrentMessage();
				if (CurrentMessage.IsValid() && CurrentMessage->InteractionId != Event.PacketId.InteractionId)
				{
					CancelCurrentInteraction();
				}
			}
		};

	ProcessTarget(Event.Routing.Target);

	for (const auto& ToActor : Event.Routing.Targets)
	{
		if (ToActor.Name != Event.Routing.Target.Name)
		{
			ProcessTarget(ToActor);
		}
	}

	const auto& FromActor = Event.Routing.Source;
	if (FromActor.Type == EInworldActorType::AGENT)
	{
		if (Event.Final)
		{
			UE_LOG(LogInworldAIIntegration, Log, TEXT("From %s: %s"), *FromActor.Name, *Event.Text);
		}

		MessageQueue->AddOrUpdateMessage<FCharacterMessageUtterance>(Event, [Event](auto MessageToUpdate) {
			MessageToUpdate->Text = Event.Text;
			MessageToUpdate->bTextFinal = Event.Final;
		});
	}
}

void UInworldCharacterComponent::VisitAudioOnClient(const FInworldAudioDataEvent& Event)
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	MessageQueue->AddOrUpdateMessage<FCharacterMessageUtterance>(Event, [Event](auto MessageToUpdate) {
		MessageToUpdate->SoundData.Append(Event.Chunk);

		ensure(!MessageToUpdate->bAudioFinal);
		MessageToUpdate->bAudioFinal = Event.bFinal;

		auto& VisemeInfos = Event.VisemeInfos;
		MessageToUpdate->VisemeInfos.Reserve(VisemeInfos.Num());
		for (auto& VisemeInfo : VisemeInfos)
		{
			FCharacterUtteranceVisemeInfo& VisemeInfo_Ref = MessageToUpdate->VisemeInfos.AddDefaulted_GetRef();
			VisemeInfo_Ref.Timestamp = VisemeInfo.Timestamp;
			VisemeInfo_Ref.Code = VisemeInfo.Code;
		}
	});
}

void UInworldCharacterComponent::OnRep_TargetPlayer(UInworldPlayerComponent* OldTargetPlayer)
{
	OnPlayerInteractionStateChanged.Broadcast(TargetPlayer != nullptr);
}

void UInworldCharacterComponent::OnRep_AgentId(FString OldAgentId)
{
	if (AgentId == OldAgentId)
	{
		return;
	}

	// BeginPlay can be called later, don't use cached ptr
	auto* InworldApi = GetWorld()->GetSubsystem<UInworldApiSubsystem>();
	if (!ensure(InworldApi))
	{
		return;
	}

	const bool bWasRegistered = !OldAgentId.IsEmpty();
	const bool bIsRegistered = !AgentId.IsEmpty();
	if (bIsRegistered && !bWasRegistered)
	{
		OnPossessed.Broadcast();
	}
	if (!bIsRegistered && bWasRegistered)
	{
		OnUnpossessed.Broadcast();
	}

	InworldApi->UpdateCharacterComponentRegistrationOnClient(this, AgentId, OldAgentId);
}

void UInworldCharacterComponent::Multicast_VisitSilence_Implementation(const FInworldSilenceEvent& Event)
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	MessageQueue->AddOrUpdateMessage<FCharacterMessageSilence>(Event, [Event](auto MessageToUpdate) {
		MessageToUpdate->Duration = Event.Duration;
	});
}

void UInworldCharacterComponent::Multicast_VisitControl_Implementation(const FInworldControlEvent& Event)
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	if (Event.Action == EInworldControlEventAction::INTERACTION_END)
	{
		MessageQueue->AddOrUpdateMessage<FCharacterMessageInteractionEnd>(Event);
	}
}

void UInworldCharacterComponent::Multicast_VisitCustom_Implementation(const FInworldCustomEvent& Event)
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	UE_LOG(LogInworldAIIntegration, Log, TEXT("CustomEvent arrived: %s - %s"), *Event.Name, *Event.PacketId.InteractionId);

	FCharacterMessageTrigger CharacterMessageTrigger;
	CharacterMessageTrigger.InteractionId = Event.PacketId.InteractionId;
	CharacterMessageTrigger.UtteranceId = Event.PacketId.UtteranceId;
	CharacterMessageTrigger.Name = Event.Name;
	CharacterMessageTrigger.Params = Event.Params.RepMap;

	OnTrigger.Broadcast(CharacterMessageTrigger);
}

void UInworldCharacterComponent::Multicast_VisitRelation_Implementation(const FInworldRelationEvent& Event)
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	FCharacterMessageTrigger CharacterMessageTrigger;
	CharacterMessageTrigger.InteractionId = Event.PacketId.InteractionId;
	CharacterMessageTrigger.UtteranceId = Event.PacketId.UtteranceId;
	CharacterMessageTrigger.Name = TEXT("inworld.relation.update");
	CharacterMessageTrigger.Params.Add(TEXT("Attraction"), FString::FromInt(Event.Attraction));
	CharacterMessageTrigger.Params.Add(TEXT("Familiar"), FString::FromInt(Event.Familiar));
	CharacterMessageTrigger.Params.Add(TEXT("Flirtatious"), FString::FromInt(Event.Flirtatious));
	CharacterMessageTrigger.Params.Add(TEXT("Respect"), FString::FromInt(Event.Respect));
	CharacterMessageTrigger.Params.Add(TEXT("Trust"), FString::FromInt(Event.Trust));

	OnTrigger.Broadcast(CharacterMessageTrigger);
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

void UInworldCharacterComponent::AddPendingInteraction(const FString& InteractionId)
{
	if (!CanceledInteractionIds.Contains(InteractionId) && !PendingInteractionIds.Contains(InteractionId))
	{
		PendingInteractionIds.Add(InteractionId);
	}
}

void UInworldCharacterComponent::Visit(const FInworldPacket& Event)
{
	AddPendingInteraction(Event.PacketId.InteractionId);
}

void UInworldCharacterComponent::Visit(const FInworldTextEvent& Event)
{
	if (CanceledInteractionIds.Contains(Event.PacketId.InteractionId))
	{
		return;
	}

	Multicast_VisitText(Event);
}

void UInworldCharacterComponent::Visit(const FInworldAudioDataEvent& Event)
{
	if (CanceledInteractionIds.Contains(Event.PacketId.InteractionId))
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

	if (ensure(InworldSubsystem.IsValid()))
	{
		TArray<FInworldAudioDataEvent> RepEvents;
		FInworldAudioDataEvent::ConvertToReplicatableEvents(Event, RepEvents);

		for (auto& E : RepEvents)
		{
			InworldSubsystem->ReplicateAudioEventFromServer(E);
		}
	}
}

void UInworldCharacterComponent::Visit(const FInworldSilenceEvent& Event)
{
	if (CanceledInteractionIds.Contains(Event.PacketId.InteractionId))
	{
		return;
	}
	Multicast_VisitSilence(Event);
}

void UInworldCharacterComponent::Visit(const FInworldControlEvent& Event)
{
	if (Event.Action == EInworldControlEventAction::INTERACTION_END)
	{
		PendingInteractionIds.Remove(Event.PacketId.InteractionId);
		CanceledInteractionIds.Remove(Event.PacketId.InteractionId);
	}

	Multicast_VisitControl(Event);
}

void UInworldCharacterComponent::Visit(const FInworldEmotionEvent& Event)
{
	if (CanceledInteractionIds.Contains(Event.PacketId.InteractionId))
	{
		return;
	}
	Multicast_VisitEmotion(Event);
}

void UInworldCharacterComponent::Visit(const FInworldCustomEvent& Event)
{
	if (CanceledInteractionIds.Contains(Event.PacketId.InteractionId))
	{
		return;
	}
	Multicast_VisitCustom(Event);
}

void UInworldCharacterComponent::Visit(const FInworldRelationEvent& Event)
{
	if (CanceledInteractionIds.Contains(Event.PacketId.InteractionId))
	{
		return;
	}
	Multicast_VisitRelation(Event);
}

void UInworldCharacterComponent::Handle(const FCharacterMessageUtterance& Message)
{
	OnUtterance.Broadcast(Message);
}

void UInworldCharacterComponent::Interrupt(const FCharacterMessageUtterance& Message)
{
	OnUtteranceInterrupt.Broadcast(Message);
}

void UInworldCharacterComponent::Handle(const FCharacterMessageSilence& Message)
{
	OnSilence.Broadcast(Message);
}

void UInworldCharacterComponent::Interrupt(const FCharacterMessageSilence& Message)
{
	OnSilenceInterrupt.Broadcast(Message);
}

void UInworldCharacterComponent::Handle(const FCharacterMessageInteractionEnd& Message)
{
	OnInteractionEnd.Broadcast(Message);
}
