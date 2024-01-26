/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "InworldCharacterComponent.h"

#include "InworldPlayerComponent.generated.h"

class UInworldApiSubsystem;
class UInworldCharacterComponent;

USTRUCT()
struct FInworldPlayerTargetCharacter
{
    GENERATED_BODY()

	FDelegateHandle UnpossessedHandle;

	UPROPERTY()
	FString AgentId;
};

UCLASS(ClassGroup = (Inworld), meta = (BlueprintSpawnableComponent))
class INWORLDAIINTEGRATION_API UInworldPlayerComponent : public UActorComponent, public Inworld::IPlayerComponent
{
	GENERATED_BODY()

public:
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnInworldPlayerTargetChange, UInworldCharacterComponent*);
    FOnInworldPlayerTargetChange OnTargetSet;
    FOnInworldPlayerTargetChange OnTargetClear;

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION(BlueprintCallable, Category = "Interaction", meta = (Displayname = "GetTargetCharacter"))
    UInworldCharacterComponent* GetTargetInworldCharacter() { return static_cast<UInworldCharacterComponent*>(GetTargetCharacter()); }

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    TArray<FString> GetTargetAgentIds();

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void ContinueMultiAgentConversation();

    virtual Inworld::ICharacterComponent* GetTargetCharacter() override;

    UFUNCTION(BlueprintCallable, Category = "Interaction", meta = (Displayname = "SetTargetCharacter"))
    void SetTargetInworldCharacter(UInworldCharacterComponent* Character);

    UFUNCTION(BlueprintCallable, Category = "Interaction", meta = (Displayname = "ClearTargetCharacter"))
    void ClearTargetInworldCharacter(UInworldCharacterComponent* Character);

    UFUNCTION(BlueprintCallable, Category = "Interaction", meta = (Displayname = "ClearTargetCharacter"))
    void ClearAllTargetInworldCharacters();

	UFUNCTION(BlueprintCallable, Category = "Interaction")
    bool IsInteracting() { return !Targets.IsEmpty(); }

    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Interaction")
    void SendTextMessageToTarget(const FString& Message);

    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Interaction")
    void SendTextMessage(const FString& Message, const FString& AgentId);

    UFUNCTION(BlueprintCallable, Category = "Interaction", meta = (AutoCreateRefTerm = "Params"))
    void SendTriggerToTarget(const FString& Name, const TMap<FString, FString>& Params);
    [[deprecated("UInworldPlayerComponent::SendCustomEventToTarget is deprecated, please use UInworldPlayerComponent::SendTriggerToTarget")]]
    void SendCustomEventToTarget(const FString& Name) { SendTriggerToTarget(Name, {}); }

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void StartAudioSessionWithTarget();

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void StopAudioSessionWithTarget();

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void SendAudioMessageToTarget(USoundWave* SoundWave);
    void SendAudioDataMessageToTarget(const TArray<uint8>& Data);
    void SendAudioDataMessageWithAECToTarget(const TArray<uint8>& InputData, const TArray<uint8>& OutputData);

private:

	UFUNCTION()
	void OnRep_Targets(const TArray<FInworldPlayerTargetCharacter>& OldTrgets);

    UPROPERTY(EditAnywhere, Category = "UI")
    FString UiName = "Player";

    TWeakObjectPtr<UInworldApiSubsystem> InworldSubsystem;

	UPROPERTY(ReplicatedUsing = OnRep_Targets)
	TArray<FInworldPlayerTargetCharacter> Targets;

#if defined(WITH_GAMEPLAY_DEBUGGER) && WITH_GAMEPLAY_DEBUGGER
    friend class FInworldGameplayDebuggerCategory;
#endif
};
