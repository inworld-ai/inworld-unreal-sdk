/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "InworldApi.h"
#include "InworldCharacter.h"
#include "Components/ActorComponent.h"

#include "InworldCharacterPlayback.h"
#include "InworldCharacterMessage.h"
#include "InworldEnums.h"
#include "InworldPackets.h"
#include "InworldSockets.h"

#include "Containers/Queue.h"

#include <GameFramework/OnlineReplStructs.h>

#include "InworldCharacterComponent.generated.h"

class UInworldPlayerComponent;
class FInternetAddr;

UCLASS(ClassGroup = (Inworld), meta = (BlueprintSpawnableComponent))
class INWORLDAIINTEGRATION_API UInworldCharacterComponent : public UActorComponent, public InworldPacketVisitor, public ICharacterMessageVisitor, public IInworldCharacterOwnerInterface
{
	GENERATED_BODY()

public:
	UInworldCharacterComponent();

	// IInworldCharacterOwnerInterface
	UInworldCharacter* GetInworldCharacter_Implementation() const { return InworldCharacter; }
	UInworldSession* GetInworldSession_Implementation() const { return InworldSession; }
	FTransform GetInworldPlayerTransform() const { return GetOwner()->GetTransform(); }
	// IInworldCharacterOwnerInterface

	virtual void InitializeComponent() override;
	virtual void UninitializeComponent() override;

	DECLARE_MULTICAST_DELEGATE(FOnInworldCharacterPossessed);
	FOnInworldCharacterPossessed OnPossessed;
	FOnInworldCharacterPossessed OnUnpossessed;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInworldCharacterPlayerInteractionStateChanged, bool, bInteracting);
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers|Interaction")
	FInworldCharacterPlayerInteractionStateChanged OnPlayerInteractionStateChanged;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInworldCharacterPlayerTalk, const FCharacterMessagePlayerTalk&, PlayerTalk);
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers|Interaction")
	FOnInworldCharacterPlayerTalk OnPlayerTalk;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInworldCharacterEmotionalBehaviorChanged, EInworldCharacterEmotionalBehavior, EmotionalBehavior, EInworldCharacterEmotionStrength, Strength);
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers|Emotion")
	FOnInworldCharacterEmotionalBehaviorChanged OnEmotionalBehaviorChanged;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInworldCharacterUtterance, const FCharacterMessageUtterance&, Utterance);
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers|Utterance")
	FOnInworldCharacterUtterance OnUtterance;
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers|Utterance")
	FOnInworldCharacterUtterance OnUtteranceInterrupt;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInworldCharacterSilence, const FCharacterMessageSilence&, Silence);
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers|Silence")
	FOnInworldCharacterSilence OnSilence;
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers|Silence")
	FOnInworldCharacterSilence OnSilenceInterrupt;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInworldCharacterTrigger, const FCharacterMessageTrigger&, Trigger);
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers|Trigger")
	FOnInworldCharacterTrigger OnTrigger;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInworldCharacterInteractionEnd, const FCharacterMessageInteractionEnd&, InteractionEnd);
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers|InteractionEnd")
	FOnInworldCharacterInteractionEnd OnInteractionEnd;

    virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type Reason);
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction);
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "Inworld")
	void SetBrainName(const FString& Name);

	UFUNCTION(BlueprintCallable, Category = "Inworld")
	const FString& GetBrainName() const { return InworldCharacter->GetBrainName(); }

    UFUNCTION(BlueprintCallable, Category = "Inworld")
	const FString& GetAgentId() const { return InworldCharacter->GetAgentId(); }

    UFUNCTION(BlueprintCallable, Category = "Inworld")
    const FString& GetGivenName() const { return InworldCharacter->GetGivenName(); }

    UFUNCTION(BlueprintCallable, Category = "Inworld")
    const FString& GetUiName() const { return UiName; }
    UFUNCTION(BlueprintCallable, Category = "Inworld")
    void SetUiName(const FString& Name) { UiName = Name; }

	UFUNCTION(BlueprintCallable, Category = "Inworld", meta = (DeterminesOutputType = "Class"))
	UInworldCharacterPlayback* GetPlayback(TSubclassOf<UInworldCharacterPlayback> Class) const;

	void HandlePacket(TSharedPtr<FInworldPacket> Packet);
	
	bool StartPlayerInteraction(UInworldPlayerComponent* Player);
	bool StopPlayerInteraction(UInworldPlayerComponent* Player);

	UFUNCTION(BlueprintCallable, Category = "Interactions")
	bool IsInteractingWithPlayer() const;

	UFUNCTION(BlueprintCallable, Category = "Emotions")
	EInworldCharacterEmotionalBehavior GetEmotionalBehavior() const { return EmotionalBehavior; }

	UFUNCTION(BlueprintPure, Category = "Emotions")
	EInworldCharacterEmotionStrength GetEmotionStrength() const { return EmotionStrength; }

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SendTextMessage(const FString& Text) const;

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Interaction", meta = (AutoCreateRefTerm = "Params"))
	void SendTrigger(const FString& Name, const TMap<FString, FString>& Params) const;
	[[deprecated("UInworldCharacterComponent::SendCustomEvent is deprecated, please use UInworldCharacterComponent::SendTrigger")]]
	void SendCustomEvent(const FString& Name) const { SendTrigger(Name, {}); }
    
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SendNarrationEvent(const FString& Content);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void StartAudioSession(const AActor* Owner) const;

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void StopAudioSession() const;

    UFUNCTION(BlueprintCallable, Category = "Interaction")
	void CancelCurrentInteraction();

	UFUNCTION(BlueprintCallable, Category = "Events")
	bool Register();

	UFUNCTION(BlueprintCallable, Category = "Events")
	bool Unregister();

	UFUNCTION(BlueprintPure, Category = "Interaction")
	FVector GetTargetPlayerCameraLocation();

	const TSharedPtr<FCharacterMessage> GetCurrentMessage() const
	{ 
		return MessageQueue->CurrentMessage;
	}

	UFUNCTION(BlueprintCallable, Category = "Message")
	void MakeMessageQueueLock(UPARAM(ref) FInworldCharacterMessageQueueLockHandle& Handle);

	UFUNCTION(BlueprintCallable, Category = "Message")
	static void ClearMessageQueueLock(UPARAM(ref) FInworldCharacterMessageQueueLockHandle& Handle);

	template<class T>
	T* GetPlaybackNative()
	{
		for (auto* Pb : Playbacks)
		{
			if (auto* Playback = Cast<T>(Pb))
			{
				return Playback;
			}
		}
		return nullptr;
	}

	UPROPERTY(EditAnywhere, Category = "Inworld")
	TArray<TSubclassOf<UInworldCharacterPlayback>> PlaybackTypes;

protected:

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	FString UiName = "Character";

private:

	virtual void Visit(const FInworldTextEvent& Event) override;
	virtual void Visit(const FInworldAudioDataEvent& Event) override;
	virtual void Visit(const FInworldSilenceEvent& Event) override;
	virtual void Visit(const FInworldControlEvent& Event) override;
	virtual void Visit(const FInworldEmotionEvent& Event) override;
	virtual void Visit(const FInworldCustomEvent& Event) override;
	virtual void Visit(const FInworldRelationEvent& Event) override;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_VisitText(const FInworldTextEvent& Event);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_VisitSilence(const FInworldSilenceEvent& Event);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_VisitControl(const FInworldControlEvent& Event);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_VisitEmotion(const FInworldEmotionEvent& Event);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_VisitCustom(const FInworldCustomEvent& Event);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_VisitRelation(const FInworldRelationEvent& Event);

	bool IsCustomGesture(const FString& CustomEventName) const;

	void VisitAudioOnClient(const FInworldAudioDataEvent& Event);

	UFUNCTION()
	void OnRep_TargetPlayer(UInworldPlayerComponent* OldPlayer);

	UFUNCTION()
	void OnRep_AgentId(FString OldAgentId);

	TQueue<FInworldAudioDataEvent> PendingRepAudioEvents;

	UPROPERTY(ReplicatedUsing = OnRep_TargetPlayer)
	UInworldPlayerComponent* TargetPlayer;
	
	TWeakObjectPtr<UInworldApiSubsystem> InworldSubsystem;

	UPROPERTY()
	TArray<UInworldCharacterPlayback*> Playbacks;

	TSharedRef<FCharacterMessageQueue> MessageQueue;
	float TimeToForceQueue = 3.f;

	virtual void Handle(const FCharacterMessageUtterance& Message) override;
	virtual void Interrupt(const FCharacterMessageUtterance& Message) override;

	virtual void Handle(const FCharacterMessageSilence& Message) override;
	virtual void Interrupt(const FCharacterMessageSilence& Message) override;

	virtual void Handle(const FCharacterMessageTrigger& Message) override;

	virtual void Handle(const FCharacterMessageInteractionEnd& Message) override;

    EInworldCharacterEmotionalBehavior EmotionalBehavior = EInworldCharacterEmotionalBehavior::NEUTRAL;
    EInworldCharacterEmotionStrength EmotionStrength = EInworldCharacterEmotionStrength::UNSPECIFIED;

	UPROPERTY(EditAnywhere, Category = "Inworld")
	FString BrainName;

	UPROPERTY(ReplicatedUsing = OnRep_AgentId)
	FString AgentId;
	
	FString GivenName;

private:
	UPROPERTY()
	UInworldCharacter* InworldCharacter;

	UPROPERTY()
	UInworldSession* InworldSession;

#if defined(WITH_GAMEPLAY_DEBUGGER) && WITH_GAMEPLAY_DEBUGGER
	friend class FInworldGameplayDebuggerCategory;
#endif
};
