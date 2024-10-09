/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "InworldSession.h"
#include "UObject/Interface.h"
#include "UObject/NoExportTypes.h"
#include "GameFramework/Actor.h"
#include "InworldTypes.h"
#include "InworldSession.h"
#include "InworldCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInworldCharacterPossessed, bool, bPossessed);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInworldCharacterPossessedNative, bool /*bPossessed*/);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInworldCharacterTargetPlayerChanged);
DECLARE_MULTICAST_DELEGATE(FOnInworldCharacterTargetPlayerChangedNative);

UCLASS(BlueprintType)
class INWORLDAICLIENT_API UInworldCharacter : public UObject
{
	GENERATED_BODY()
public:
	UInworldCharacter();
	virtual ~UInworldCharacter();

	// UObject
	virtual UWorld* GetWorld() const override { return GetTypedOuter<AActor>()->GetWorld(); }
	virtual void BeginDestroy() { SetSession(nullptr); Super::BeginDestroy(); }
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual int32 GetFunctionCallspace(UFunction* Function, FFrame* Stack) override;
	virtual bool CallRemoteFunction(UFunction* Function, void* Parms, struct FOutParmRec* OutParms, FFrame* Stack) override;
	// ~UObject

public:
	/**
	 * Handle a packet.
	 * @param WrappedPacket The wrapped packet to handle.
	 */
	UFUNCTION()
	void HandlePacket(const FInworldWrappedPacket& WrappedPacket);

	/**
	 * Set the session.
	 * @param InSession The session to set.
	 */
	UFUNCTION(BlueprintCallable, Category="Session")
	void SetSession(UInworldSession* InSession);

	/**
	 * Get the session.
	 * @return The session.
	 */
	UFUNCTION(BlueprintPure, Category = "Session")
	UInworldSession* GetSession() const { return Session.Get(); }

	/**
	 * Send a text message.
	 * @param Text The text message to send.
	 */
	UFUNCTION(BlueprintCallable, Category = "Message|Text")
	void SendTextMessage(const FString& Text);

	/**
	 * Send a trigger message.
	 * @param Name The name of the trigger.
	 * @param Params The parameters for the trigger.
	 */
	UFUNCTION(BlueprintCallable, Category = "Message|Trigger")
	void SendTrigger(const FString& Name, const TMap<FString, FString>& Params);

	/**
	 * Send a narration event.
	 * @param Content The content of the narration event.
	 */
	UFUNCTION(BlueprintCallable, Category = "Message|Narration")
	void SendNarrationEvent(const FString& Content);

	/**
	 * Start an audio session.
	 * @param SessionOptions The options for the audio session.
	 */
	UFUNCTION(BlueprintCallable, Category = "Message|Audio")
	void SendAudioSessionStart(FInworldAudioSessionOptions SessionOptions);

	/**
	 * Stop the audio session.
	 */
	UFUNCTION(BlueprintCallable, Category = "Message|Audio")
	void SendAudioSessionStop();

	/**
	 * Send a sound message.
	 * @param Input The input sound data.
	 * @param Output The output sound data.
	 */
	UFUNCTION(BlueprintCallable, Category = "Message|Audio")
	void SendSoundMessage(const TArray<uint8>& Input, const TArray<uint8>& Output);

	/**
	 * Cancel a response.
	 * @param InteractionId The ID of the interaction to cancel.
	 * @param UtteranceIds The IDs of the utterances to cancel.
	 */
	UFUNCTION(BlueprintCallable, Category = "Message|Mutation")
	void CancelResponse(const FString& InteractionId, const TArray<FString>& UtteranceIds);

public:
	/**
	 * Set the brain name.
	 * @param BrainName The name of the brain to set.
	 */
	UFUNCTION(BlueprintCallable, Category = "Possession")
	void SetBrainName(const FString& BrainName);

	/**
	 * Check if the character is possessed.
	 * @return True if possessed, false otherwise.
	 */
	UFUNCTION(BlueprintPure, Category = "Possession")
	bool IsPossessed() const { return !AgentInfo.AgentId.IsEmpty(); }

	/**
	 * Possess the character with the given agent info.
	 * @param InAgentInfo The agent info to possess the character with.
	 */
	UFUNCTION(BlueprintCallable, Category = "Possession")
	void Possess(const FInworldAgentInfo& InAgentInfo);

	/**
	 * Unpossess the character.
	 */
	UFUNCTION(BlueprintCallable, Category = "Possession")
	void Unpossess();

	/**
	 * Get the agent info of the character.
	 * @return The agent info of the character.
	 */
	UFUNCTION(BlueprintPure, Category = "Possession")
	const FInworldAgentInfo& GetAgentInfo() const { return AgentInfo; }

	/**
	 * Delegate for when the character is possessed.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Possession")
	FOnInworldCharacterPossessed OnPossessedDelegate;
	FOnInworldCharacterPossessedNative& OnPossessed() { return OnPossessedDelegateNative; }

	/**
	 * Set the target player.
	 * @param Player The player to set as the target.
	 */
	UFUNCTION(BlueprintCallable, Category = "Target")
	void SetTargetPlayer(UInworldPlayer* Player);

	/**
	 * Clear the target player.
	 */
	UFUNCTION(BlueprintCallable, Category = "Target")
	void ClearTargetPlayer();

	/**
	 * Get the target player.
	 * @return The target player.
	 */
	UFUNCTION(BlueprintPure, Category = "Target")
	UInworldPlayer* GetTargetPlayer() const { return TargetPlayer; }

	/**
	 * Delegate for when the target player is changed.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Target")
	FOnInworldCharacterTargetPlayerChanged OnTargetPlayerChangedDelegate;
	FOnInworldCharacterTargetPlayerChangedNative& OnTargetPlayerChanged() { return OnTargetPlayerChangedDelegateNative; }

	/**
	 * Delegate for Inworld text events.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Event")
	FOnInworldTextEvent OnInworldTextEventDelegate;
	FOnInworldTextEventNative& OnInworldTextEvent() { return OnInworldTextEventDelegateNative; }

	/**
	 * Delegate for Inworld VAD events.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Event")
	FOnInworldVADEvent OnInworldVADEventDelegate;
	FOnInworldVADEventNative& OnInworldVADEvent() { return OnInworldVADEventDelegateNative; }

	/**
	 * Delegate for Inworld audio events.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Event")
	FOnInworldAudioEvent OnInworldAudioEventDelegate;
	FOnInworldAudioEventNative& OnInworldAudioEvent() { return OnInworldAudioEventDelegateNative; }

	/**
	 * Delegate for Inworld A2F header events.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Event")
	FOnInworldA2FHeaderEvent OnInworldA2FHeaderEventDelegate;
	FOnInworldA2FHeaderEventNative& OnInworldA2FHeaderEvent() { return OnInworldA2FHeaderEventDelegateNative; }

	/**
	 * Delegate for Inworld A2F content events.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Event")
	FOnInworldA2FContentEvent OnInworldA2FContentEventDelegate;
	FOnInworldA2FContentEventNative& OnInworldA2FContentEvent() { return OnInworldA2FContentEventDelegateNative; }

	/**
	 * Delegate for Inworld silence events.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Event")
	FOnInworldSilenceEvent OnInworldSilenceEventDelegate;
	FOnInworldSilenceEventNative& OnInworldSilenceEvent() { return OnInworldSilenceEventDelegateNative; }

	/**
	 * Delegate for Inworld control events.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Event")
	FOnInworldControlEvent OnInworldControlEventDelegate;
	FOnInworldControlEventNative& OnInworldControlEvent() { return OnInworldControlEventDelegateNative; }

	/**
	 * Delegate for Inworld emotion events.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Event")
	FOnInworldEmotionEvent OnInworldEmotionEventDelegate;
	FOnInworldEmotionEventNative& OnInworldEmotionEvent() { return OnInworldEmotionEventDelegateNative; }

	/**
	 * Delegate for  Inworld custom events.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Event")
	FOnInworldCustomEvent OnInworldCustomEventDelegate;
	FOnInworldCustomEventNative& OnInworldCustomEvent() { return OnInworldCustomEventDelegateNative; }

private:
	UPROPERTY(Replicated)
	TWeakObjectPtr<UInworldSession> Session;

	UPROPERTY(Replicated)
	FInworldAgentInfo AgentInfo;
	FOnInworldCharacterPossessedNative OnPossessedDelegateNative;

	UFUNCTION()
	void OnRep_TargetPlayer(UInworldPlayer* OldTargetPlayer);

	UPROPERTY(ReplicatedUsing=OnRep_TargetPlayer)
	UInworldPlayer* TargetPlayer;
	FOnInworldCharacterTargetPlayerChangedNative OnTargetPlayerChangedDelegateNative;

	FOnInworldTextEventNative OnInworldTextEventDelegateNative;
	FOnInworldVADEventNative OnInworldVADEventDelegateNative;
	FOnInworldAudioEventNative OnInworldAudioEventDelegateNative;
	FOnInworldA2FHeaderEventNative OnInworldA2FHeaderEventDelegateNative;
	FOnInworldA2FContentEventNative OnInworldA2FContentEventDelegateNative;
	FOnInworldSilenceEventNative OnInworldSilenceEventDelegateNative;
	FOnInworldControlEventNative OnInworldControlEventDelegateNative;
	FOnInworldEmotionEventNative OnInworldEmotionEventDelegateNative;
	FOnInworldCustomEventNative OnInworldCustomEventDelegateNative;

	class FInworldCharacterPacketVisitor : public TSharedFromThis<FInworldCharacterPacketVisitor>, public InworldPacketVisitor
	{
	public:
		FInworldCharacterPacketVisitor()
			: FInworldCharacterPacketVisitor(nullptr)
		{}
		FInworldCharacterPacketVisitor(class UInworldCharacter* InCharacter)
			: Character(InCharacter)
		{}
		virtual ~FInworldCharacterPacketVisitor() = default;

		virtual void Visit(const FInworldTextEvent& Event) override;
		virtual void Visit(const FInworldVADEvent& Event) override;
		virtual void Visit(const FInworldAudioDataEvent& Event) override;
		virtual void Visit(const FInworldA2FHeaderEvent& Event) override;
		virtual void Visit(const FInworldA2FContentEvent& Event) override;
		virtual void Visit(const FInworldSilenceEvent& Event) override;
		virtual void Visit(const FInworldControlEvent& Event) override;
		virtual void Visit(const FInworldEmotionEvent& Event) override;
		virtual void Visit(const FInworldCustomEvent& Event) override;

	private:
		UInworldCharacter* Character;
	};
	
	TSharedRef<FInworldCharacterPacketVisitor> PacketVisitor;
};

UINTERFACE(MinimalAPI, BlueprintType)
class UInworldCharacterOwnerInterface : public UInterface
{
	GENERATED_BODY()
};

class INWORLDAICLIENT_API IInworldCharacterOwnerInterface
{
	GENERATED_BODY()

public:
	/**
	 * Get the Inworld Character.
	 * @return A pointer to the Inworld Character.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inworld")
	UInworldCharacter* GetInworldCharacter() const;
};
