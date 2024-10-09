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
class INWORLDAIINTEGRATION_API UInworldCharacterComponent : public UActorComponent, public IInworldCharacterOwnerInterface, public ICharacterMessageVisitor
{
	GENERATED_BODY()

public:
	UInworldCharacterComponent();

	// IInworldCharacterOwnerInterface
	virtual UInworldCharacter* GetInworldCharacter_Implementation() const override { return InworldCharacter; }
	// ~IInworldCharacterOwnerInterface

	virtual void OnRegister() override;
	virtual void OnUnregister() override;
	virtual void InitializeComponent() override;
	virtual void UninitializeComponent() override;

	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type Reason);
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInworldCharacterPlayerInteractionStateChanged, bool, bInteracting);
	/**
	 * Event dispatcher for when the interaction state of the Inworld character with the player changes.
	 */
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers|Interaction")
	FInworldCharacterPlayerInteractionStateChanged OnPlayerInteractionStateChanged;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInworldCharacterPlayerTalk, const FCharacterMessagePlayerTalk&, PlayerTalk);
	/**
	 * Event dispatcher for when the player talks with the Inworld character.
	 */
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers|Interaction")
	FOnInworldCharacterPlayerTalk OnPlayerTalk;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInworldCharacterEmotionalBehaviorChanged, EInworldCharacterEmotionalBehavior, EmotionalBehavior, EInworldCharacterEmotionStrength, Strength);
	/**
	 * Event dispatcher for when the emotional behavior of the Inworld character changes.
	 */
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers|Emotion")
	FOnInworldCharacterEmotionalBehaviorChanged OnEmotionalBehaviorChanged;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInworldCharacterUtterance, const FCharacterMessageUtterance&, Utterance);
	/**
	 * Event dispatcher for when the Inworld character utters a message.
	 */
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers|Utterance")
	FOnInworldCharacterUtterance OnUtterance;

	/**
	 * Event dispatcher for when the Inworld character's utterance is interrupted.
	 */
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers|Utterance")
	FOnInworldCharacterUtterance OnUtteranceInterrupt;

	/**
	 * Event dispatcher for when the Inworld character's utterance is paused.
	 */
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers|Utterance")
	FOnInworldCharacterUtterance OnUtterancePause;

	/**
	 * Event dispatcher for when the Inworld character's utterance resumes.
	 */
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers|Utterance")
	FOnInworldCharacterUtterance OnUtteranceResume;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInworldCharacterSilence, const FCharacterMessageSilence&, Silence);
	/**
	 * Event dispatcher for when the Inworld character is silenced.
	 */
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers|Silence")
	FOnInworldCharacterSilence OnSilence;

	/**
	 * Event dispatcher for when the Inworld character's silence is interrupted.
	 */
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers|Silence")
	FOnInworldCharacterSilence OnSilenceInterrupt;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInworldCharacterTrigger, const FCharacterMessageTrigger&, Trigger);
	/**
	 * Event dispatcher for when a trigger event is received from the Inworld character.
	 */
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers|Trigger")
	FOnInworldCharacterTrigger OnTrigger;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInworldCharacterInteractionEnd, const FCharacterMessageInteractionEnd&, InteractionEnd);
	/**
	 * Event dispatcher for when an interaction with the Inworld character ends.
	 */
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers|InteractionEnd")
	FOnInworldCharacterInteractionEnd OnInteractionEnd;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInworldPlayerVoiceDetection, bool, bVoiceDetected);
	/**
	 * Event dispatcher for detecting voice from the Inworld player.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Conversation")
	FOnInworldPlayerVoiceDetection OnVoiceDetection;

	/**
	 * Sets the name of the brain.
	 * @param Name The name to set for the brain.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inworld")
	void SetBrainName(const FString& Name);

	/**
	 * Gets the name of the brain.
	 * @return The name of the brain.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inworld")
	FString GetBrainName() const;

	/**
	 * Gets the agent ID.
	 * @return The agent ID.
	 */
    UFUNCTION(BlueprintCallable, Category = "Inworld")
	FString GetAgentId() const;

	/**
	 * Gets the given name.
	 * @return The given name.
	 */
    UFUNCTION(BlueprintCallable, Category = "Inworld")
	FString GetGivenName() const;

	/**
	 * Gets the UI name.
	 * @return The UI name.
	 */
    UFUNCTION(BlueprintCallable, Category = "Inworld")
    const FString& GetUiName() const { return UiName; }
	/**
	 * Sets the UI name.
	 * @param Name The name to set for the UI.
	 */
    UFUNCTION(BlueprintCallable, Category = "Inworld")
    void SetUiName(const FString& Name) { UiName = Name; }

	/**
	 * Gets the playback instance of the specified class.
	 * @param Class The class of the playback instance.
	 * @return The playback instance.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inworld", meta = (DeterminesOutputType = "Class"))
	UInworldCharacterPlayback* GetPlayback(TSubclassOf<UInworldCharacterPlayback> Class) const;

	/**
	 * Checks if the character is interacting with a player.
	 * @return True if interacting with a player, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "Interactions")
	bool IsInteractingWithPlayer() const;

	/**
	 * Gets the emotional behavior of the character.
	 * @return The emotional behavior.
	 */
	UFUNCTION(BlueprintCallable, Category = "Emotions")
	EInworldCharacterEmotionalBehavior GetEmotionalBehavior() const { return EmotionalBehavior; }

	/**
	 * Gets the emotion strength of the character.
	 * @return The emotion strength.
	 */
	UFUNCTION(BlueprintPure, Category = "Emotions")
	EInworldCharacterEmotionStrength GetEmotionStrength() const { return EmotionStrength; }

	/**
	 * Sends a text message.
	 * @param Text The text message to send.
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SendTextMessage(const FString& Text) const;

	/**
	 * Sends a trigger with parameters.
	 * @param Name The name of the trigger.
	 * @param Params The parameters for the trigger.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Interaction", meta = (AutoCreateRefTerm = "Params"))
	void SendTrigger(const FString& Name, const TMap<FString, FString>& Params) const;
	[[deprecated("UInworldCharacterComponent::SendCustomEvent is deprecated, please use UInworldCharacterComponent::SendTrigger")]]
	void SendCustomEvent(const FString& Name) const { SendTrigger(Name, {}); }
    
	/**
	 * Sends a narration event with the specified content.
	 * @param Content The content of the narration event.
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SendNarrationEvent(const FString& Content);

	/**
	 * Starts an audio session with the provided options.
	 * @param SessionOptions The options for the audio session.
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void StartAudioSession(FInworldAudioSessionOptions SessionOptions);

	/**
	 * Stops the current audio session.
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void StopAudioSession();

	/**
	 * Retrieves the target player camera location.
	 * @return The location of the target player camera.
	 */
	UFUNCTION(BlueprintPure, Category = "Interaction")
	FVector GetTargetPlayerCameraLocation();

	/**
	 * Interrupts the current interaction.
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void Interrupt();

	void Interrupt(const FString& InteractionId);

	const TSharedPtr<FCharacterMessage> GetCurrentMessage() const
	{ 
		return MessageQueue->CurrentMessageQueueEntry ? MessageQueue->CurrentMessageQueueEntry->GetCharacterMessage() : nullptr;
	}

	/**
	 * Locks the message queue using the provided handle.
	 * @param Handle The reference to the handle used to lock the message queue.
	 * @return True if the message queue was successfully locked, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "Message")
	bool LockMessageQueue(UPARAM(ref) FInworldCharacterMessageQueueLockHandle& Handle) { return MessageQueue->Lock(Handle); }

	/**
	 * Unlocks the message queue using the provided handle.
	 * @param Handle The reference to the handle used to unlock the message queue.
	 */
	UFUNCTION(BlueprintCallable, Category = "Message")
	void UnlockMessageQueue(UPARAM(ref) FInworldCharacterMessageQueueLockHandle& Handle) { MessageQueue->Unlock(Handle); }

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

	/**
	 * Array of playback types for Inworld characters.
	 */
	UPROPERTY(EditAnywhere, Category = "Inworld")
	TArray<TSubclassOf<UInworldCharacterPlayback>> PlaybackTypes;

protected:
	/**
	 * Flag indicating whether to find a session.
	 */
	UPROPERTY(EditInstanceOnly, Category = "Inworld")
	bool bFindSession = true;

	/**
	 * Inworld session owner (hidden if bFindSession is false).
	 * Must implement the InworldSessionOwnerInterface.
	 * @see /Script/InworldAIClient.InworldSessionOwnerInterface
	 */
	UPROPERTY(EditInstanceOnly, Category = "Inworld", meta = (EditCondition = "!bFindSession", EditConditionHides, MustImplement = "/Script/InworldAIClient.InworldSessionOwnerInterface"))
	AActor* InworldSessionOwner;

	/**
	 * The name of the UI element.
	 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	FString UiName = "Character";

private:
	UFUNCTION()
	void OnInworldTextEvent(const FInworldTextEvent& Event);
	UFUNCTION()
	void OnInworldVADEvent(const FInworldVADEvent& Event);
	UFUNCTION()
	void OnInworldAudioEvent(const FInworldAudioDataEvent& Event);
	UFUNCTION()
	void OnInworldA2FHeaderEvent(const FInworldA2FHeaderEvent& Event);
	UFUNCTION()
	void OnInworldA2FContentEvent(const FInworldA2FContentEvent& Event);
	UFUNCTION()
	void OnInworldSilenceEvent(const FInworldSilenceEvent& Event);
	UFUNCTION()
	void OnInworldControlEvent(const FInworldControlEvent& Event);
	UFUNCTION()
	void OnInworldEmotionEvent(const FInworldEmotionEvent& Event);
	UFUNCTION()
	void OnInworldCustomEvent(const FInworldCustomEvent& Event);
	UFUNCTION()
	void OnInworldRelationEvent(const FInworldRelationEvent& Event);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_VisitText(const FInworldTextEvent& Event);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_VisitVAD(const FInworldVADEvent& Event);
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

	TQueue<FInworldAudioDataEvent> PendingRepAudioEvents;

	UPROPERTY()
	TArray<UInworldCharacterPlayback*> Playbacks;

	TSharedRef<FCharacterMessageQueue> MessageQueue;

	virtual void Handle(const FCharacterMessageUtterance& Message) override;
	virtual void Interrupt(const FCharacterMessageUtterance& Message) override;
	virtual void Pause(const FCharacterMessageUtterance& Event) override;
	virtual void Resume(const FCharacterMessageUtterance& Event) override;

	virtual void Handle(const FCharacterMessageSilence& Message) override;
	virtual void Interrupt(const FCharacterMessageSilence& Message) override;

	virtual void Handle(const FCharacterMessageTrigger& Message) override;

	virtual void Handle(const FCharacterMessageInteractionEnd& Message) override;

	TMap<FString, TArray<FString>> PendingCancelResponses;

    EInworldCharacterEmotionalBehavior EmotionalBehavior = EInworldCharacterEmotionalBehavior::NEUTRAL;
    EInworldCharacterEmotionStrength EmotionStrength = EInworldCharacterEmotionStrength::UNSPECIFIED;

	UPROPERTY(EditAnywhere, Category = "Inworld")
	FString BrainName;

private:
	UFUNCTION()
	void OnRep_InworldCharacter();

	UPROPERTY(ReplicatedUsing=OnRep_InworldCharacter)
	UInworldCharacter* InworldCharacter;

#if defined(WITH_GAMEPLAY_DEBUGGER) && WITH_GAMEPLAY_DEBUGGER
	friend class FInworldGameplayDebuggerCategory;
#endif
};
