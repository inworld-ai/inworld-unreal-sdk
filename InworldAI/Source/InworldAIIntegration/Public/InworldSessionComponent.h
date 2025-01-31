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

UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
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

	UFUNCTION(BlueprintNativeEvent, Category = "Connection")
	void HandleUndefinedReconnection(const FString& OutErrorMessage, const int32 OutErrorCode);
	
	/**
	 * Start a session.
	 * @param SceneId The scene to initialize.
	 * @param Save The save data.
	 * @param SessionToken The session token.
	 */
	UFUNCTION(BlueprintCallable, Category = "Session", meta = (DisplayName = "StartSession", AdvancedDisplay = "1", AutoCreateRefTerm = "Save, Token", DeprecatedFunction, DeprecationMessage = "UInworldSessionComponent::StartSession has been depricated, please use StartSessionFrom(Scene/Save/Token)."))
	void StartSession(const FString& SceneId, const FInworldSave& Save, const FInworldToken& Token);

	/**
	 * Start a session from a scene.
	 * @param Scene The scene to initialize.
	 */
	UFUNCTION(BlueprintCallable, Category = "Session")
	void StartSessionFromScene(const FInworldScene& Scene);
  
  /**
	 * Start a session from a save.
	 * @param Save The save data.
	 */
	UFUNCTION(BlueprintCallable, Category = "Session")
	void StartSessionFromSave(const FInworldSave& Save);
  
  /**
	 * Start a session from a token.
	 * @param SessionToken The session token.
	 */
	UFUNCTION(BlueprintCallable, Category = "Session")
	void StartSessionFromToken(const FInworldToken& Token);

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
	 * Get the session Token.
	 * @return The session Token.
	 */
	UFUNCTION(BlueprintPure, Category = "Session")
	FInworldToken GetSessionToken() const;
	/**
	 * Get the session ID.
	 * @return The session ID.
	 */
	UFUNCTION(BlueprintPure, Category = "Session")
	FString GetSessionId() const;

	/**
	 * Set the Player Profile.
	 * @param InPlayerProfile The Player Profile.
	 */
	UFUNCTION(BlueprintSetter)
	void SetPlayerProfile(const FInworldPlayerProfile& InPlayerProfile);

	/**
	 * Set the Capability Set.
	 * @param InCapabilitySet The Capability Set.
	 */
	UFUNCTION(BlueprintSetter)
	void SetCapabilitySet(const FInworldCapabilitySet& InCapabilitySet);

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
	 * Workspace configuration.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Config")
	FString Workspace;

	/**
	 * Authentication configuration.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Config")
	FInworldAuth Auth;

	/**
	 * Player Profile configuration.
	 */
	UPROPERTY(EditAnywhere, BlueprintSetter=SetPlayerProfile, Category = "Config")
	FInworldPlayerProfile PlayerProfile;

	/**
	 * CapabilitySet configuration.
	 */
	UPROPERTY(EditAnywhere, BlueprintSetter=SetCapabilitySet, Category = "Config")
	FInworldCapabilitySet CapabilitySet;

	/**
	 * Metadata map for additional configuration details.
	 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Config")
	TMap<FString, FString> Metadata;

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
