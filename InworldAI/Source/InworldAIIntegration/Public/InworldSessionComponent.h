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

	/**
	 * Delegate for handling session creation events.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Session")
	FOnInworldSessionCreated OnSessionCreatedDelegate;
	FOnInworldSessionCreatedNative& OnSessionCreated() { return OnSessionCreatedDelegateNative; }

	/**
	 * Check if the session is loaded.
	 * @return True if the session is loaded, false otherwise.
	 */
	UFUNCTION(BlueprintPure, Category = "Load")
	bool GetIsLoaded() const;

	/**
	 * Delegate for handling session loaded events.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Connection")
	FOnInworldSessionLoaded OnSessionLoadedDelegate;
	FOnInworldSessionLoadedNative& OnLoaded() { return OnSessionLoadedDelegateNative; }

	/**
	 * Start a new session.
	 * @param SceneId The ID of the scene for the session.
	 * @param Save The save data for the session.
	 * @param Token The session token.
	 */
	UFUNCTION(BlueprintCallable, Category = "Session", meta = (DisplayName = "StartSession", AdvancedDisplay = "1", AutoCreateRefTerm = "Save, Token"))
	void StartSession(const FString& SceneId, const FInworldSave& Save, const FInworldSessionToken& Token);
	/**
	 * Stop the current session.
	 */
	UFUNCTION(BlueprintCallable, Category = "Session")
	void StopSession();
	/**
	 * Pause the current session.
	 */
	UFUNCTION(BlueprintCallable, Category = "Session")
	void PauseSession();
	/**
	 * Resume the paused session.
	 */
	UFUNCTION(BlueprintCallable, Category = "Session")
	void ResumeSession();

	/**
	 * Get the session ID.
	 * @return The session ID.
	 */
	UFUNCTION(BlueprintPure, Category = "Session")
	FString GetSessionId() const;

	/**
	 * Save the session with a callback.
	 * @param Callback The callback to be executed after saving the session.
	 */
	UFUNCTION(BlueprintCallable, Category = "Session")
	void SaveSession(FOnInworldSessionSavedCallback Callback);

	/**
	 * Send interaction feedback.
	 * @param InteractionId The ID of the interaction.
	 * @param bIsLike Whether the interaction is liked or not.
	 * @param Message The feedback message.
	 */
	UFUNCTION(BlueprintCallable, Category = "Session")
	void SendInteractionFeedback(const FString& InteractionId, bool bIsLike, const FString& Message);

	/**
	 * Delegate for handling session connection state changes.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Connection")
	FOnInworldConnectionStateChanged OnSessionConnectionStateChangedDelegate;
	FOnInworldConnectionStateChangedNative& OnSessionConnectionStateChanged() { return OnSessionConnectionStateChangedDelegateNative; }

	/**
	 * Get the connection state.
	 * @return The connection state.
	 */
	UFUNCTION(BlueprintPure, Category = "Connection")
	EInworldConnectionState GetConnectionState() const;
	/**
	 * Get the connection error details.
	 * @param OutErrorMessage The error message to output.
	 * @param OutErrorCode The error code to output.
	 * @param OutErrorDetails The error details to output.
	 */
	UFUNCTION(BlueprintPure, Category = "Connection")
	void GetConnectionError(FString& OutErrorMessage, int32& OutErrorCode, FInworldConnectionErrorDetails& OutErrorDetails) const;

protected:
	/**
	 * Player profile configuration.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Config")
	FInworldPlayerProfile PlayerProfile;

	/**
	 * Authentication configuration.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Config")
	FInworldAuth Auth;

	/**
	 * Capability set configuration.
	 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Config")
	FInworldCapabilitySet CapabilitySet;

	/**
	 * Metadata map for additional configuration details.
	 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Config")
	TMap<FString, FString> Metadata;

	/**
	 * Environment configuration for internal settings.
	 */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Config|Internal")
	FInworldEnvironment Environment;

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
