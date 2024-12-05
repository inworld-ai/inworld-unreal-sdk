/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "InworldEnums.h"
#include "UObject/Interface.h"
#include "UObject/NoExportTypes.h"
#include "GameFramework/Actor.h"
#include "InworldEnums.h"
#include "InworldTypes.h"
#include "InworldPackets.h"
#include "InworldPlayer.generated.h"

class UInworldSession;
class UInworldCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInworldPlayerTargetCharacterAdded, UInworldCharacter*, Character);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInworldPlayerTargetCharacterAddedNative, UInworldCharacter* /*Character*/);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInworldPlayerTargetCharacterRemoved, UInworldCharacter*, Character);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInworldPlayerTargetCharacterRemovedNative, UInworldCharacter* /*Character*/);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInworldPlayerTargetCharactersChanged);
DECLARE_MULTICAST_DELEGATE(FOnInworldPlayerTargetCharactersChangedNative);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInworldPlayerConversationChanged);
DECLARE_MULTICAST_DELEGATE(FOnInworldPlayerConversationChangedNative);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInworldPlayerVoiceDetection, bool, bVoiceDetected);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInworldPlayerVoiceDetectionNative, bool /*bVoiceDetected*/);

UCLASS(BlueprintType)
class INWORLDAICLIENT_API UInworldPlayer : public UObject
{
	GENERATED_BODY()
public:
	UInworldPlayer();
	virtual ~UInworldPlayer();

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
	 * Handle the incoming packet.
	 * @param WrappedPacket The wrapped packet to handle.
	 */
	UFUNCTION()
	void HandlePacket(const FInworldWrappedPacket& WrappedPacket);

	/**
	 * Set the session.
	 * @param InSession The session to set.
	 */
	UFUNCTION(BlueprintCallable, Category = "Session")
	void SetSession(UInworldSession* InSession);

	/**
	 * Get the session.
	 * @return The session.
	 */
	UFUNCTION(BlueprintPure, Category = "Session")
	UInworldSession* GetSession() const { return Session.Get(); }

	/**
	 * Send a text message to the conversation.
	 * @param Text The text message to send.
	 */
	UFUNCTION(BlueprintCallable, Category = "Message|Text")
	void SendTextMessageToConversation(const FString& Text);

	/**
	 * Send a trigger message to the conversation.
	 * @param Name The name of the trigger.
	 * @param Params The parameters associated with the trigger.
	 */
	UFUNCTION(BlueprintCallable, Category = "Message|Trigger")
	void SendTriggerToConversation(const FString& Name, const TMap<FString, FString>& Params);

	/**
	 * Start an audio session in the conversation.
	 * @param AudioSessionOptions The audio session options.
	 */
	UFUNCTION(BlueprintCallable, Category = "Message|Audio")
	void SendAudioSessionStartToConversation(FInworldAudioSessionOptions AudioSessionOptions);

	/**
	 * Stop the current audio session in the conversation.
	 */
	UFUNCTION(BlueprintCallable, Category = "Message|Audio")
	void SendAudioSessionStopToConversation();

	/**
	 * Send a sound message to the conversation.
	 * @param Input The input sound data.
	 * @param Output The output sound data.
	 */
	UFUNCTION(BlueprintCallable, Category = "Message|Audio")
	void SendSoundMessageToConversation(const TArray<uint8>& Input, const TArray<uint8>& Output);

public:
	/**
	 * Set whether the character is participating in the conversation.
	 * @param bParticipate Whether the character should participate in the conversation.
	 */
	UFUNCTION(BlueprintCallable, Category = "Participation")
	void SetConversationParticipation(bool bParticipate);

	/**
	 * Check if the character is a participant in the conversation.
	 * @return True if the character is a participant, false otherwise.
	 */
	UFUNCTION(BlueprintPure, Category = "Participation")
	bool IsConversationParticipant() const { return bConversationParticipant; }

	/**
	 * Get the target characters of the conversation.
	 * @return An array of target characters.
	 */
	UFUNCTION(BlueprintPure, Category = "Target")
	const TArray<UInworldCharacter*>& GetTargetCharacters() const { return TargetCharacters; }

	/**
	 * Add a target character to the conversation.
	 * @param TargetCharacter The target character to add.
	 */
	UFUNCTION(BlueprintCallable, Category = "Target")
	void AddTargetCharacter(UInworldCharacter* TargetCharacter);

	/**
	 * Remove a target character from the conversation.
	 * @param TargetCharacter The target character to remove.
	 */
	UFUNCTION(BlueprintCallable, Category = "Target")
	void RemoveTargetCharacter(UInworldCharacter* TargetCharacter);

	/**
	 * Clear all target characters from the conversation.
	 */
	UFUNCTION(BlueprintCallable, Category = "Target")
	void ClearAllTargetCharacters();

	/**
	 * Delegate for when a target character is added.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Target")
	FOnInworldPlayerTargetCharacterAdded OnTargetCharacterAddedDelegate;
	FOnInworldPlayerTargetCharacterAddedNative& OnTargetCharacterAdded() { return OnTargetCharacterAddedDelegateNative; }

	/**
	 * Delegate for when a target character is removed.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Target")
	FOnInworldPlayerTargetCharacterRemoved OnTargetCharacterRemovedDelegate;
	FOnInworldPlayerTargetCharacterRemovedNative& OnTargetCharacterRemoved() { return OnTargetCharacterRemovedDelegateNative; }

	/**
	 * Delegate for when target characters are changed.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Target")
	FOnInworldPlayerTargetCharactersChanged OnTargetCharactersChangedDelegate;
	FOnInworldPlayerTargetCharactersChangedNative& OnTargetCharactersChanged() { return OnTargetCharactersChangedDelegateNative; }

	/**
	 * Get the conversation ID.
	 * @return The conversation ID.
	 */
	UFUNCTION(BlueprintPure, Category = "Conversation")
	const FString& GetConversationId() const { return ConversationId; }

	/**
	 * Delegate for when the conversation changes.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Conversation")
	FOnInworldPlayerConversationChanged OnConversationChangedDelegate;
	FOnInworldPlayerConversationChangedNative& OnConversationChanged() { return OnConversationChangedDelegateNative; }

	/**
	 * Delegate for voice detection events.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Conversation")
	FOnInworldPlayerVoiceDetection OnVoiceDetectionDelegate;
	FOnInworldPlayerVoiceDetectionNative& OnVoiceDetection() { return OnVoiceDetectionDelegateNative; }

	bool HasAudioSession() const { return bHasAudioSession; }
	EInworldMicrophoneMode GetMicMode() const { return AudioSessionOptions.MicrophoneMode; }

private:
	void UpdateConversation();

private:
	UFUNCTION()
	void OnRep_VoiceDetected(bool bOldValue);
	
	UPROPERTY(Replicated)
	TWeakObjectPtr<UInworldSession> Session;

	UPROPERTY(Replicated)
	bool bConversationParticipant = true;

	UPROPERTY(Replicated)
	TArray<UInworldCharacter*> TargetCharacters;

	UPROPERTY(ReplicatedUsing=OnRep_VoiceDetected)
	bool bVoiceDetected = false;

	FOnInworldPlayerTargetCharacterAddedNative OnTargetCharacterAddedDelegateNative;
	FOnInworldPlayerTargetCharacterRemovedNative OnTargetCharacterRemovedDelegateNative;
	FOnInworldPlayerTargetCharactersChangedNative OnTargetCharactersChangedDelegateNative;
	FOnInworldPlayerVoiceDetectionNative OnVoiceDetectionDelegateNative;

	FString ConversationId;
	FOnInworldPlayerConversationChangedNative OnConversationChangedDelegateNative;

	FInworldAudioSessionOptions AudioSessionOptions;
	bool bHasAudioSession = false;

	class FInworldPlayerPacketVisitor : public TSharedFromThis<FInworldPlayerPacketVisitor>, public InworldPacketVisitor
	{
	public:
		FInworldPlayerPacketVisitor()
			: FInworldPlayerPacketVisitor(nullptr)
		{}
		FInworldPlayerPacketVisitor(class UInworldPlayer* InPlayer)
			: Player(InPlayer)
		{}
		virtual ~FInworldPlayerPacketVisitor() = default;

		virtual void Visit(const FInworldVADEvent& Event) override;
		virtual void Visit(const FInworldConversationUpdateEvent& Event) override;

	private:
		UInworldPlayer* Player;
	};

	TSharedRef<FInworldPlayerPacketVisitor> PacketVisitor;
};

UINTERFACE(MinimalAPI, BlueprintType)
class UInworldPlayerOwnerInterface : public UInterface
{
	GENERATED_BODY()
};

class INWORLDAICLIENT_API IInworldPlayerOwnerInterface
{
	GENERATED_BODY()

public:
	/**
	 * Get the Inworld Player associated with this interface.
	 * @return The Inworld Player object.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inworld")
	UInworldPlayer* GetInworldPlayer() const;
};
