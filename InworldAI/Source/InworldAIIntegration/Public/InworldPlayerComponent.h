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

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void SetConversationParticipation(bool bParticipant);

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void ContinueConversation();

    UFUNCTION(BlueprintCallable, Category = "Interaction", meta = (Displayname = "GetTargetCharacter", DeprecatedFunction, DeprecationMessage = "Please use GetTargetCharacter"))
    UInworldCharacterComponent* GetTargetInworldCharacter();
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    UInworldCharacter* GetTargetCharacter();
    UFUNCTION(BlueprintCallable, Category = "Interaction", meta = (Displayname = "GetTargetCharacters", DeprecatedFunction, DeprecationMessage = "Please use GetTargetCharacters"))
    TArray<UInworldCharacterComponent*> GetTargetInworldCharacters();
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    TArray<UInworldCharacter*> GetTargetCharacters();

    UFUNCTION(BlueprintCallable, Category = "Interaction", meta = (Displayname = "SetTargetCharacter", DeprecatedFunction, DeprecationMessage="Please use AddTargetCharacter"))
    void SetTargetInworldCharacter(UInworldCharacterComponent* Character) { AddTargetInworldCharacter(Character); }
    UFUNCTION(BlueprintCallable, Category = "Interaction", meta = (Displayname = "AddTargetCharacter", DeprecatedFunction, DeprecationMessage = "Please use RemoveTargetCharacter"))
    void AddTargetInworldCharacter(UInworldCharacterComponent* Character);
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void AddTargetCharacter(UInworldCharacter* Character);

    UFUNCTION(BlueprintCallable, Category = "Interaction", meta = (Displayname = "ClearTargetCharacter", DeprecatedFunction, DeprecationMessage = "Please use RemoveTargetCharacter"))
    void ClearTargetInworldCharacter(UInworldCharacterComponent* Character) { RemoveTargetInworldCharacter(Character); }
    UFUNCTION(BlueprintCallable, Category = "Interaction", meta = (Displayname = "RemoveTargetCharacter", DeprecatedFunction, DeprecationMessage = "Please use RemoveTargetCharacter"))
    void RemoveTargetInworldCharacter(UInworldCharacterComponent* Character);
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void RemoveTargetCharacter(UInworldCharacter* Character);

    UFUNCTION(BlueprintCallable, Category = "Interaction", meta = (Displayname = "ClearAllTargetCharacters"))
    void ClearAllTargetInworldCharacters();

	UFUNCTION(BlueprintCallable, Category = "Interaction")
    bool IsInteracting() { return InworldPlayer->GetTargetCharacters().Num() > 0; }

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void SendTextMessageToTarget(const FString& Message);

    UFUNCTION(BlueprintCallable, Category = "Interaction", meta = (AutoCreateRefTerm = "Params"))
    void SendTriggerToTarget(const FString& Name, const TMap<FString, FString>& Params);
    [[deprecated("UInworldPlayerComponent::SendCustomEventToTarget is deprecated, please use UInworldPlayerComponent::SendTriggerToTarget")]]
    void SendCustomEventToTarget(const FString& Name) { SendTriggerToTarget(Name, {}); }

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void StartAudioSessionWithTarget(FInworldAudioSessionOptions AudioSessionOptions);

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void StopAudioSessionWithTarget();

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void SendAudioMessageToTarget(const TArray<uint8>& InputData, const TArray<uint8>& OutputData);

protected:
    UPROPERTY(EditInstanceOnly, Category = "Inworld")
    bool bFindSession = true;

    UPROPERTY(EditInstanceOnly, Category = "Inworld", meta = (EditCondition = "!bFindSession", EditConditionHides, MustImplement = "/Script/InworldAIIntegration.InworldSessionOwnerInterface"))
    AActor* InworldSessionOwner;

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
