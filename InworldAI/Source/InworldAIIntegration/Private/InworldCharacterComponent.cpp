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
	: Super()
	, MessageQueue(MakeShared<FCharacterMessageQueue>(this))
{
    PrimaryComponentTick.bCanEverTick = true;
    bWantsInitializeComponent = true;
}

void UInworldCharacterComponent::InitializeComponent()
{
    Super::InitializeComponent();

	InworldCharacter = NewObject<UInworldCharacter>(this, "InworldCharacter");
	InworldCharacter->OnPossessed().AddLambda(
		[this](bool bPossessed) -> void
		{
			AgentInfo = InworldCharacter->GetAgentInfo();
		}
	);
	InworldCharacter->OnTargetPlayerChanged().AddLambda(
		[this]() -> void
		{
			//TODO: FIX
			// set player id and whatnot
		}
	);

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

	InworldCharacter = nullptr;

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

	InworldSession = IInworldSessionOwnerInterface::Execute_GetInworldSession(GetWorld()->GetSubsystem<UInworldApiSubsystem>());
	InworldSession->OnInworldTextEvent().AddUObject(this, &UInworldCharacterComponent::OnInworldTextEvent);
	InworldSession->OnInworldAudioEvent().AddUObject(this, &UInworldCharacterComponent::OnInworldAudioEvent);
	InworldSession->OnInworldSilenceEvent().AddUObject(this, &UInworldCharacterComponent::OnInworldSilenceEvent);
	InworldSession->OnInworldControlEvent().AddUObject(this, &UInworldCharacterComponent::OnInworldControlEvent);
	InworldSession->OnInworldEmotionEvent().AddUObject(this, &UInworldCharacterComponent::OnInworldEmotionEvent);
	InworldSession->OnInworldCustomEvent().AddUObject(this, &UInworldCharacterComponent::OnInworldCustomEvent);

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
		//TODO: FIX
		//if (InworldSubsystem.IsValid())
		{
			//FString NewAgentId = FString();
			//InworldSubsystem->UpdateCharacterComponentRegistrationOnClient(this, NewAgentId, AgentId);
		}
	}
	else
	{
		Unregister();
	}
	
	MessageQueue->Clear();

    Super::EndPlay(Reason);
}

void UInworldCharacterComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	TOptional<float> BlockingTimestamp = MessageQueue->GetBlockingTimestamp();
	if (BlockingTimestamp.IsSet() && GetWorld()->GetTimeSeconds() - BlockingTimestamp.GetValue() > TimeToForceQueue)
	{
		MessageQueue->TryToProgress(true);
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
	DOREPLIFETIME(UInworldCharacterComponent, AgentInfo);
}

void UInworldCharacterComponent::SetBrainName(const FString& Name)
{
	InworldCharacter->SetBrainName(BrainName);
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

void UInworldCharacterComponent::CancelCurrentInteraction()
{
	TSharedPtr<FCharacterMessage> CurrentMessage = GetCurrentMessage();
    if (!ensure(CurrentMessage.IsValid()))
    {
        return;
    }

	const FString CurrentInteractionId = CurrentMessage->InteractionId;
	TArray<FString> CanceledUtterances = MessageQueue->CancelInteraction(CurrentInteractionId);
	if (CanceledUtterances.Num() > 0)
	{
		InworldSession->CancelResponse(InworldCharacter, CurrentInteractionId, CanceledUtterances);
	}
}

void UInworldCharacterComponent::SendTextMessage(const FString& Text) const
{
	InworldSession->SendTextMessage(InworldCharacter, Text);
}

void UInworldCharacterComponent::SendTrigger(const FString& Name, const TMap<FString, FString>& Params) const
{
	InworldSession->SendTrigger(InworldCharacter, Name, Params);
}

void UInworldCharacterComponent::SendNarrationEvent(const FString& Content)
{
	InworldSession->SendNarrationEvent(InworldCharacter, Content);
}

void UInworldCharacterComponent::StartAudioSession(const AActor* Owner) const
{
	InworldSession->SendAudioSessionStart(InworldCharacter);
}

void UInworldCharacterComponent::StopAudioSession() const
{
	InworldSession->SendAudioSessionStop(InworldCharacter);
}

bool UInworldCharacterComponent::Register()
{
    if (BrainName.IsEmpty())
    {
        return false;
    }

	InworldCharacter->SetBrainName(BrainName);

    return true;
}

bool UInworldCharacterComponent::Unregister()
{
	if (BrainName.IsEmpty())
	{
		return false;
	}

	InworldCharacter->SetBrainName({});

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

void UInworldCharacterComponent::MakeMessageQueueLock(FInworldCharacterMessageQueueLockHandle& Handle)
{
	Handle.Lock = MessageQueue->MakeLock();
}

void UInworldCharacterComponent::ClearMessageQueueLock(FInworldCharacterMessageQueueLockHandle& Handle)
{
	Handle.Lock = nullptr;
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

		MessageQueue->AddOrUpdateMessage<FCharacterMessageUtterance>(Event, GetWorld()->GetTimeSeconds(), [Event](auto MessageToUpdate) {
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

	MessageQueue->AddOrUpdateMessage<FCharacterMessageUtterance>(Event, GetWorld()->GetTimeSeconds(), [Event](auto MessageToUpdate) {
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

void UInworldCharacterComponent::OnRep_AgentInfo(FInworldAgentInfo OldAgentId)
{
	InworldCharacter->Possess(AgentInfo);
}

void UInworldCharacterComponent::Multicast_VisitSilence_Implementation(const FInworldSilenceEvent& Event)
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	MessageQueue->AddOrUpdateMessage<FCharacterMessageSilence>(Event, GetWorld()->GetTimeSeconds(), [Event](auto MessageToUpdate) {
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
		MessageQueue->AddOrUpdateMessage<FCharacterMessageInteractionEnd>(Event, GetWorld()->GetTimeSeconds());
	}
}

void UInworldCharacterComponent::Multicast_VisitCustom_Implementation(const FInworldCustomEvent& Event)
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	UE_LOG(LogInworldAIIntegration, Log, TEXT("CustomEvent arrived: %s - %s"), *Event.Name, *Event.PacketId.InteractionId);

	MessageQueue->AddOrUpdateMessage<FCharacterMessageTrigger>(Event, GetWorld()->GetTimeSeconds(), [Event](auto MessageToUpdate) {
		MessageToUpdate->Name = Event.Name;
		MessageToUpdate->Params = Event.Params.RepMap;
	});
}

void UInworldCharacterComponent::Multicast_VisitRelation_Implementation(const FInworldRelationEvent& Event)
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	MessageQueue->AddOrUpdateMessage<FCharacterMessageTrigger>(Event, GetWorld()->GetTimeSeconds(), [Event](auto MessageToUpdate) {
		MessageToUpdate->Name = TEXT("inworld.relation.update");
		MessageToUpdate->Params.Add(TEXT("Attraction"), FString::FromInt(Event.Attraction));
		MessageToUpdate->Params.Add(TEXT("Familiar"), FString::FromInt(Event.Familiar));
		MessageToUpdate->Params.Add(TEXT("Flirtatious"), FString::FromInt(Event.Flirtatious));
		MessageToUpdate->Params.Add(TEXT("Respect"), FString::FromInt(Event.Respect));
		MessageToUpdate->Params.Add(TEXT("Trust"), FString::FromInt(Event.Trust));
	});
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
	if (Event.Routing.Source.Name != GetAgentId())
	{
		return;
	}
    Multicast_VisitText(Event);
}

void UInworldCharacterComponent::OnInworldAudioEvent(const FInworldAudioDataEvent& Event)
{
	if (Event.Routing.Source.Name != GetAgentId())
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

void UInworldCharacterComponent::OnInworldSilenceEvent(const FInworldSilenceEvent& Event)
{
	if (Event.Routing.Source.Name != GetAgentId())
	{
		return;
	}
    Multicast_VisitSilence(Event);
}

void UInworldCharacterComponent::OnInworldControlEvent(const FInworldControlEvent& Event)
{
	if (Event.Routing.Source.Name != GetAgentId())
	{
		return;
	}
    Multicast_VisitControl(Event);
}

void UInworldCharacterComponent::OnInworldEmotionEvent(const FInworldEmotionEvent& Event)
{
	if (Event.Routing.Source.Name != GetAgentId())
	{
		return;
	}
    Multicast_VisitEmotion(Event);
}

void UInworldCharacterComponent::OnInworldCustomEvent(const FInworldCustomEvent& Event)
{
	if (Event.Routing.Source.Name != GetAgentId())
	{
		return;
	}
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

void UInworldCharacterComponent::Handle(const FCharacterMessageTrigger& Message)
{
	OnTrigger.Broadcast(Message);
}

void UInworldCharacterComponent::Handle(const FCharacterMessageInteractionEnd& Message)
{
	OnInteractionEnd.Broadcast(Message);
}
