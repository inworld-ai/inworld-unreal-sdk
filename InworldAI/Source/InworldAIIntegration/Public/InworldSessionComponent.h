/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InworldSession.h"
#include "InworldSessionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInworldSessionCreated);
DECLARE_MULTICAST_DELEGATE(FOnInworldSessionCreatedNative);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class INWORLDAIINTEGRATION_API UInworldSessionComponent : public UActorComponent, public IInworldSessionOwnerInterface
{
	GENERATED_BODY()

public:	
	UInworldSessionComponent();

	// IInworldSessionOwnerInterface
	virtual UInworldSession* GetInworldSession_Implementation() const override { return InworldSession; }
	// ~IInworldSessionOwnerInterface

	virtual void OnRegister() override;
	virtual void OnUnregister() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

	UPROPERTY(BlueprintAssignable, Category = "Session")
	FOnInworldSessionCreated OnSessionCreatedDelegate;
	FOnInworldSessionCreatedNative& OnSessionCreated() { return OnSessionCreatedDelegateNative; }

	UFUNCTION(BlueprintPure, Category = "Load")
	bool GetIsLoaded() const;

	UPROPERTY(BlueprintAssignable, Category = "Connection")
	FOnInworldSessionLoaded OnSessionLoadedDelegate;
	FOnInworldSessionLoadedNative& OnLoaded() { return OnSessionLoadedDelegateNative; }

	UFUNCTION(BlueprintCallable, Category = "Session")
	void StartSession();
	UFUNCTION(BlueprintCallable, Category = "Session")
	void StartSessionFromSave(const FInworldSave& Save);
	UFUNCTION(BlueprintCallable, Category = "Session")
	void StartSessionFromToken(const FInworldSessionToken& Token);
	UFUNCTION(BlueprintCallable, Category = "Session")
	void StopSession();
	UFUNCTION(BlueprintCallable, Category = "Session")
	void PauseSession();
	UFUNCTION(BlueprintCallable, Category = "Session")
	void ResumeSession();

	UFUNCTION(BlueprintPure, Category = "Session")
	FString GetSessionId() const;

	UFUNCTION(BlueprintCallable, Category = "Session")
	void SaveSession(FOnInworldSessionSavedCallback Callback);

	UFUNCTION(BlueprintCallable, Category = "Session")
	void SendInteractionFeedback(const FString& InteractionId, bool bIsLike, const FString& Message);

	UPROPERTY(BlueprintAssignable, Category = "Connection")
	FOnInworldConnectionStateChanged OnSessionConnectionStateChangedDelegate;
	FOnInworldConnectionStateChangedNative& OnSessionConnectionStateChanged() { return OnSessionConnectionStateChangedDelegateNative; }

	UFUNCTION(BlueprintPure, Category = "Connection")
	EInworldConnectionState GetConnectionState() const;
	UFUNCTION(BlueprintPure, Category = "Connection")
	void GetConnectionError(FString& OutErrorMessage, int32& OutErrorCode) const;

	UFUNCTION(BlueprintCallable, Category = "Scene")
	void SetSceneId(const FString& InSceneId);

	UFUNCTION(BlueprintCallable, Category = "Player Profile")
	void SetPlayerProfile(const FInworldPlayerProfile& InPlayerProfile);

	UFUNCTION(BlueprintCallable, Category = "Capabilities")
	void SetCapabilities(const FInworldCapabilitySet& InCapabilitySet);

protected:
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Config")
	FString SceneId;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Config")
	FInworldPlayerProfile PlayerProfile;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Config")
	FInworldAuth Auth;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Config")
	FInworldCapabilitySet CapabilitySet;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Config|Internal")
	FInworldEnvironment Environment;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Config|Connection")
	float RetryConnectionIntervalTime = 0.25f;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Config|Connection")
	float MaxRetryConnectionTime = 5.0f;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Config|Connection")
	float CurrentRetryConnectionTime = 1.0f;

	FTimerHandle RetryConnectionTimerHandle;

private:
	FOnInworldSessionCreatedNative OnSessionCreatedDelegateNative;
	FOnInworldSessionLoadedNative OnSessionLoadedDelegateNative;
	FOnInworldConnectionStateChangedNative OnSessionConnectionStateChangedDelegateNative;

	UFUNCTION()
	void OnRep_InworldSession();

	UPROPERTY(ReplicatedUsing = OnRep_InworldSession)
	UInworldSession* InworldSession;
};
