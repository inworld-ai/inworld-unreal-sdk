/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "InworldCharacterComponent.h"
#include "InworldPlayer.h"

#include "InworldPlayerComponent.generated.h"

class UInworldApiSubsystem;
class UInworldCharacterComponent;

UCLASS(ClassGroup = (Inworld), meta = (BlueprintSpawnableComponent))
class INWORLDAIINTEGRATION_API UInworldPlayerComponent : public UActorComponent, public IInworldPlayerOwnerInterface
{
    GENERATED_BODY()

public:
    UInworldPlayerComponent();

    // IInworldPlayerInterface
    virtual UInworldPlayer* GetInworldPlayer_Implementation() const override { return InworldPlayer; }
    // ~IInworldPlayerInterface

    virtual void OnRegister() override;
    virtual void OnUnregister() override;
    virtual void InitializeComponent() override;
    virtual void UninitializeComponent() override;

    virtual void BeginPlay() override;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

    /**
     * Set whether the player is participating in a conversation.
     * @param bParticipant Whether the player is a participant in the conversation.
     */
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void SetConversationParticipation(bool bParticipant);

    /**
     * Continue the current conversation.
     */
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void ContinueConversation();

    /**
     * Get the target Inworld character (deprecated, please use GetTargetCharacter).
     * @return The target Inworld character.
     */
    UFUNCTION(BlueprintCallable, Category = "Interaction", meta = (Displayname = "GetTargetCharacter", DeprecatedFunction, DeprecationMessage = "Please use GetTargetCharacter"))
    UInworldCharacterComponent* GetTargetInworldCharacter();
    /**
     * Get the target character.
     * @return The target character.
     */
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    UInworldCharacter* GetTargetCharacter();

    /**
     * Get a list of target Inworld characters (deprecated, please use GetTargetCharacters).
     * @return An array of target Inworld characters.
     */
    UFUNCTION(BlueprintCallable, Category = "Interaction", meta = (Displayname = "GetTargetCharacters", DeprecatedFunction, DeprecationMessage = "Please use GetTargetCharacters"))
    TArray<UInworldCharacterComponent*> GetTargetInworldCharacters();

    /**
     * Get a list of target characters.
     * @return An array of target characters.
     */
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    TArray<UInworldCharacter*> GetTargetCharacters();

    /**
     * Set the target Inworld character (deprecated, please use AddTargetCharacter).
     * @param Character The character to set as the target.
     */
    UFUNCTION(BlueprintCallable, Category = "Interaction", meta = (Displayname = "SetTargetCharacter", DeprecatedFunction, DeprecationMessage="Please use AddTargetCharacter"))
    void SetTargetInworldCharacter(UInworldCharacterComponent* Character) { AddTargetInworldCharacter(Character); }

    /**
     * Add a target Inworld character (deprecated, please use RemoveTargetCharacter).
     * @param Character The character to add as a target.
     */
    UFUNCTION(BlueprintCallable, Category = "Interaction", meta = (Displayname = "AddTargetCharacter", DeprecatedFunction, DeprecationMessage = "Please use RemoveTargetCharacter"))
    void AddTargetInworldCharacter(UInworldCharacterComponent* Character);

    /**
     * Add a target character.
     * @param Character The character to add as a target.
     */
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void AddTargetCharacter(UInworldCharacter* Character);

    /**
     * Clear the target Inworld character (deprecated, please use RemoveTargetCharacter).
     * @param Character The character to clear as the target.
     */
    UFUNCTION(BlueprintCallable, Category = "Interaction", meta = (Displayname = "ClearTargetCharacter", DeprecatedFunction, DeprecationMessage = "Please use RemoveTargetCharacter"))
    void ClearTargetInworldCharacter(UInworldCharacterComponent* Character) { RemoveTargetInworldCharacter(Character); }

    /**
     * Remove a target Inworld character (deprecated, please use RemoveTargetCharacter).
     * @param Character The character to remove as a target.
     */
    UFUNCTION(BlueprintCallable, Category = "Interaction", meta = (Displayname = "RemoveTargetCharacter", DeprecatedFunction, DeprecationMessage = "Please use RemoveTargetCharacter"))
    void RemoveTargetInworldCharacter(UInworldCharacterComponent* Character);

    /**
     * Remove a target character.
     * @param Character The character to remove as a target.
     */
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void RemoveTargetCharacter(UInworldCharacter* Character);


    /**
     * Clear all target Inworld characters.
     */
    UFUNCTION(BlueprintCallable, Category = "Interaction", meta = (Displayname = "ClearAllTargetCharacters"))
    void ClearAllTargetInworldCharacters();

    /**
     * Check if the player is currently interacting with any characters.
     * @return True if the player is interacting with characters, false otherwise.
     */
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    bool IsInteracting() { return InworldPlayer->GetTargetCharacters().Num() > 0; }

    /**
     * Send a text message to the target character.
     * @param Message The message to send.
     */
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void SendTextMessageToTarget(const FString& Message);

    /**
     * Send a trigger to the target character with parameters.
     * @param Name The name of the trigger.
     * @param Params The parameters for the trigger.
     */
    UFUNCTION(BlueprintCallable, Category = "Interaction", meta = (AutoCreateRefTerm = "Params"))
    void SendTriggerToTarget(const FString& Name, const TMap<FString, FString>& Params);
    [[deprecated("UInworldPlayerComponent::SendCustomEventToTarget is deprecated, please use UInworldPlayerComponent::SendTriggerToTarget")]]
    void SendCustomEventToTarget(const FString& Name) { SendTriggerToTarget(Name, {}); }

    /**
     * Start an audio session with the target character using the specified audio session options.
     * @param AudioSessionOptions The audio session options.
     */
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void StartAudioSessionWithTarget(FInworldAudioSessionOptions AudioSessionOptions);

    /**
     * Stop the audio session with the target character.
     */
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void StopAudioSessionWithTarget();

    /**
     * Send an audio message to the target character.
     * @param InputData The input audio data.
     * @param OutputData The output audio data.
     */
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void SendAudioMessageToTarget(const TArray<uint8>& InputData, const TArray<uint8>& OutputData);

protected:
    /**
     * Whether to find the session.
     */
    UPROPERTY(EditInstanceOnly, Category = "Inworld")
    bool bFindSession = true;

    /**
     * Inworld session owner (hidden if bFindSession is false).
     * Must implement the InworldSessionOwnerInterface.
     * @see /Script/InworldAIIntegration.InworldSessionOwnerInterface
     */
    UPROPERTY(EditInstanceOnly, Category = "Inworld", meta = (EditCondition = "!bFindSession", EditConditionHides, MustImplement = "/Script/InworldAIIntegration.InworldSessionOwnerInterface"))
    AActor* InworldSessionOwner;

    /**
     * UI name.
     */
    UPROPERTY(EditAnywhere, Category = "UI")
    FString UiName = "Player";

private:
    UPROPERTY(Replicated)
    UInworldPlayer* InworldPlayer;

    UPROPERTY(EditDefaultsOnly, Category = "Conversation")
    bool bConversationParticipant = true;

#if defined(WITH_GAMEPLAY_DEBUGGER) && WITH_GAMEPLAY_DEBUGGER
    friend class FInworldGameplayDebuggerCategory;
#endif
};
