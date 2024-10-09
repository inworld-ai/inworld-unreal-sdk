/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "UObject/NoExportTypes.h"
#include "GameFramework/Actor.h"
#include "InworldClient.h"
#include "InworldTypes.h"
#include "InworldPackets.h"
#include "InworldEnums.h"
#include "InworldSession.generated.h"

class UInworldPlayer;
class UInworldCharacter;
class UInworldClient;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInworldTextEvent, const FInworldTextEvent&, TextEvent);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInworldTextEventNative, const FInworldTextEvent& /*TextEvent*/);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInworldVADEvent, const FInworldVADEvent&, VADEvent);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInworldVADEventNative, const FInworldVADEvent& /*VADEvent*/);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInworldAudioEvent, const FInworldAudioDataEvent&, AudioEvent);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInworldAudioEventNative, const FInworldAudioDataEvent& /*AudioEvent*/);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInworldA2FHeaderEvent, const FInworldA2FHeaderEvent&, A2FHeaderEvent);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInworldA2FHeaderEventNative, const FInworldA2FHeaderEvent& /*A2FHeaderEvent*/);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInworldA2FContentEvent, const FInworldA2FContentEvent&, A2FContentEvent);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInworldA2FContentEventNative, const FInworldA2FContentEvent& /*A2FContentEvent*/);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInworldSilenceEvent, const FInworldSilenceEvent&, SilenceEvent);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInworldSilenceEventNative, const FInworldSilenceEvent& /*SilenceEvent*/);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInworldControlEvent, const FInworldControlEvent&, ControlEvent);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInworldControlEventNative, const FInworldControlEvent& /*ControlEvent*/);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInworldConversationUpdateEvent, const FInworldConversationUpdateEvent&, ConversationUpdateEvent);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInworldConversationUpdateEventNative, const FInworldConversationUpdateEvent& /*ConversationUpdateEvent*/);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInworldEmotionEvent, const FInworldEmotionEvent&, EmotionEvent);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInworldEmotionEventNative, const FInworldEmotionEvent& /*EmotionEvent*/);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInworldCustomEvent, const FInworldCustomEvent&, CustomEvent);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInworldCustomEventNative, const FInworldCustomEvent& /*CustomEvent*/);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInworldSessionLoaded, bool, bLoaded);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInworldSessionLoadedNative, bool /*bLoaded*/);

UCLASS(BlueprintType)
class INWORLDAICLIENT_API UInworldSession : public UObject
{
	GENERATED_BODY()
public:
	UInworldSession();
	virtual ~UInworldSession();

	// UObject
	virtual UWorld* GetWorld() const override { return GetTypedOuter<AActor>()->GetWorld(); }
	virtual void BeginDestroy() { Destroy(); Super::BeginDestroy(); }
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual int32 GetFunctionCallspace(UFunction* Function, FFrame* Stack) override;
	virtual bool CallRemoteFunction(UFunction* Function, void* Parms, struct FOutParmRec* OutParms, FFrame* Stack) override;
	// ~UObject

public:
	/**
	 * Initialize the client.
	 */
	UFUNCTION(BlueprintCallable, Category = "Client")
	void Init();

	/**
	 * Destroy the client.
	 */
	UFUNCTION(BlueprintCallable, Category = "Client")
	void Destroy();

	/**
	 * Get the client.
	 * @return The Inworld client.
	 */
	UFUNCTION(BlueprintPure, Category = "Client")
	UInworldClient* GetClient() const { return Client; }

	/**
	 * Handle an incoming packet.
	 * @param WrappedPacket The wrapped packet to handle.
	 */
	UFUNCTION()
	void HandlePacket(const FInworldWrappedPacket& WrappedPacket);

	/**
	 * Register a character.
	 * @param Character The character to register.
	 */
	UFUNCTION(BlueprintCallable, Category = "Register")
	void RegisterCharacter(UInworldCharacter* Character);

	/**
	 * Unregister a character.
	 * @param Character The character to unregister.
	 */
	UFUNCTION(BlueprintCallable, Category = "Register")
	void UnregisterCharacter(UInworldCharacter* Character);

	/**
	 * Get the registered characters.
	 * @return An array of registered characters.
	 */
	UFUNCTION(BlueprintPure, Category = "Register")
	const TArray<UInworldCharacter*>& GetRegisteredCharacters() const { return RegisteredCharacters; }

	/**
	 * Register a player.
	 * @param Player The player to register.
	 */
	UFUNCTION(BlueprintCallable, Category = "Register")
	void RegisterPlayer(UInworldPlayer* Player);

	/**
	 * Unregister a player.
	 * @param Player The player to unregister.
	 */
	UFUNCTION(BlueprintCallable, Category = "Register")
	void UnregisterPlayer(UInworldPlayer* Player);

	/**
	 * Get the registered players.
	 * @return An array of registered players.
	 */
	UFUNCTION(BlueprintPure, Category = "Register")
	const TArray<UInworldPlayer*>& GetRegisteredPlayers() const { return RegisteredPlayers; }

    /**
	 * Start a session from a scene.
	 * @param Scene The scene to initialize.
	 * @param PlayerProfile The player's profile.
	 * @param CapabilitySet The capability set.
	 * @param Metadata Additional metadata.
	 * @param WorkspaceOverride The workspace to use instead of the project default.
	 * @param AuthOverride The authentication to use instead of project default.
	 */
	UFUNCTION(BlueprintCallable, Category = "Session", meta = (AdvancedDisplay = "1", AutoCreateRefTerm = "PlayerProfile, CapabilitySet, Metadata, AuthOverride"))
	void StartSessionFromScene(const FInworldScene& Scene, const FInworldPlayerProfile& PlayerProfile, const FInworldCapabilitySet& CapabilitySet, const TMap<FString, FString>& Metadata, const FString& WorkspaceOverride, const FInworldAuth& AuthOverride);
	
    /**
	 * Start a session from a save.
	 * @param Save The save data.
	 * @param PlayerProfile The player's profile.
	 * @param CapabilitySet The capability set.
	 * @param Metadata Additional metadata.
	 * @param WorkspaceOverride The workspace to use instead of the project default.
	 * @param AuthOverride The authentication to use instead of project default.
	 */
    UFUNCTION(BlueprintCallable, Category = "Session", meta = (AdvancedDisplay = "1", AutoCreateRefTerm = "PlayerProfile, CapabilitySet, Metadata, AuthOverride"))
	void StartSessionFromSave(const FInworldSave& Save, const FInworldPlayerProfile& PlayerProfile, const FInworldCapabilitySet& CapabilitySet, const TMap<FString, FString>& Metadata, const FString& WorkspaceOverride, const FInworldAuth& AuthOverride);
	
    /**
	* Start a session from a token.
	* @param SessionToken The session token.
	* @param PlayerProfile The player's profile.
	* @param CapabilitySet The capability set.
	* @param Metadata Additional metadata.
	* @param WorkspaceOverride The workspace to use instead of the project default.
	* @param AuthOverride The authentication to use instead of project default.
	*/
    UFUNCTION(BlueprintCallable, Category = "Session", meta = (AdvancedDisplay = "1", AutoCreateRefTerm = "PlayerProfile, CapabilitySet, Metadata, AuthOverride"))
	void StartSessionFromToken(const FInworldToken& Token, const FInworldPlayerProfile& PlayerProfile, const FInworldCapabilitySet& CapabilitySet, const TMap<FString, FString>& Metadata, const FString& WorkspaceOverride, const FInworldAuth& AuthOverride);
	
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
	 * Get the current session token.
	 * @return The session token.
	 */
	UFUNCTION(BlueprintPure, Category = "Session")
	FInworldToken GetSessionToken() const;

	/**
	 * Get the current session ID.
	 * @return The session ID.
	 */
	UFUNCTION(BlueprintPure, Category = "Session")
	FString GetSessionId() const;
  
	/**
	 * Load the player profile.
	 * @param The player profile to load.
	 */
	UFUNCTION(BlueprintCallable, Category = "Load|Configuration")
	void LoadPlayerProfile(const FInworldPlayerProfile& PlayerProfile);

	/**
	 * Get the current session capabilities.
	 * @return The session capabilities.
	 */
	UFUNCTION(BlueprintPure, Category = "Session")
	FInworldCapabilitySet GetCapabilities() const;
  
    /**
	 * Load the capabilities.
	 * @param The player profile to load.
	 */
	UFUNCTION(BlueprintCallable, Category = "Load|Configuration")
	void LoadCapabilities(const FInworldCapabilitySet& CapabilitySet);

	/**
	 * Save the current session.
	 * @param Callback The callback function to execute after saving.
	 */
	UFUNCTION(BlueprintCallable, Category = "Session")
	void SaveSession(FOnInworldSessionSavedCallback Callback);

	/**
	 * Send feedback for a specific interaction.
	 * @param InteractionId The ID of the interaction.
	 * @param bIsLike Whether the feedback is positive or negative.
	 * @param Message The feedback message.
	 */
	UFUNCTION(BlueprintCallable, Category = "Session")
	void SendInteractionFeedback(const FString& InteractionId, bool bIsLike, const FString& Message);

	/**
	 * Load a character.
	 * @param Character The character to load.
	 */
	UFUNCTION(BlueprintCallable, Category = "Load|Character")
	void LoadCharacter(UInworldCharacter* Character) { LoadCharacters({ Character }); }
	/**
	 * Load multiple characters.
	 * @param Characters An array of characters to load.
	 */
	UFUNCTION(BlueprintCallable, Category = "Load|Character")
	void LoadCharacters(const TArray<UInworldCharacter*>& Characters);
	/**
	 * Unload a character.
	 * @param Character The character to unload.
	 */
	UFUNCTION(BlueprintCallable, Category = "Load|Character")
	void UnloadCharacter(UInworldCharacter* Character) { UnloadCharacters({ Character }); }
	/**
	 * Unload multiple characters.
	 * @param Characters An array of characters to unload.
	 */
	UFUNCTION(BlueprintCallable, Category = "Load|Character")
	void UnloadCharacters(const TArray<UInworldCharacter*>& Characters);

	/**
	 * Update a conversation for a player.
	 * @param Player The player to update the conversation for.
	 * @return The updated conversation.
	 */
	UFUNCTION(BlueprintCallable, Category = "Conversation")
	FString UpdateConversation(UInworldPlayer* Player);

	/**
	 * Send a text message to a character.
	 * @param Character The character to send the message to.
	 * @param Message The text message to send.
	 */
	UFUNCTION(BlueprintCallable, Category = "Message|Text")
	void SendTextMessage(UInworldCharacter* Character, const FString& Message);
	/**
	 * Send a text message to a conversation for a player.
	 * @param Player The player to send the message to.
	 * @param Message The text message to send.
	 */
	UFUNCTION(BlueprintCallable, Category = "Message|Text")
	void SendTextMessageToConversation(UInworldPlayer* Player, const FString& Message);

	/**
	 * Initialize the speech processor with the specified mode and options.
	 * @param Mode The speech mode to initialize.
	 * @param SpeechOptions The speech options to set.
	 */
	UFUNCTION(BlueprintCallable, Category = "Message|Audio")
	void InitSpeechProcessor(EInworldPlayerSpeechMode Mode, const FInworldPlayerSpeechOptions& SpeechOptions);
	/**
	 * Destroy the speech processor.
	 */
	UFUNCTION(BlueprintCallable, Category = "Message|Audio")
	void DestroySpeechProcessor();

	/**
	 * Send a sound message to a character.
	 * @param Character The character to send the sound message to.
	 * @param InputData The input sound data.
	 * @param OutputData The output sound data.
	 */
	UFUNCTION(BlueprintCallable, Category = "Message|Audio")
	void SendSoundMessage(UInworldCharacter* Character, const TArray<uint8>& InputData, const TArray<uint8>& OutputData);
	/**
	 * Send a sound message to a conversation for a player.
	 * @param Player The player to send the sound message to.
	 * @param InputData The input sound data.
	 * @param OutputData The output sound data.
	 */
	UFUNCTION(BlueprintCallable, Category = "Message|Audio")
	void SendSoundMessageToConversation(UInworldPlayer* Player, const TArray<uint8>& InputData, const TArray<uint8>& OutputData);

	/**
	 * Start an audio session for a character with the specified options.
	 * @param Character The character to start the audio session for.
	 * @param SessionOptions The audio session options.
	 */
	UFUNCTION(BlueprintCallable, Category = "Message|Audio")
	void SendAudioSessionStart(UInworldCharacter* Character, FInworldAudioSessionOptions SessionOptions);
	/**
	 * Start an audio session for a conversation for a player with the specified options.
	 * @param Player The player to start the audio session for.
	 * @param SessionOptions The audio session options.
	 */
	UFUNCTION(BlueprintCallable, Category = "Message|Audio")
	void SendAudioSessionStartToConversation(UInworldPlayer* Player, FInworldAudioSessionOptions SessionOptions);

	/**
	 * Stop the audio session for a character.
	 * @param Character The character to stop the audio session for.
	 */
	UFUNCTION(BlueprintCallable, Category = "Message|Audio")
	void SendAudioSessionStop(UInworldCharacter* Character);

	/**
	 * Stop the audio session for a conversation for a player.
	 * @param Player The player to stop the audio session for.
	 */
	UFUNCTION(BlueprintCallable, Category = "Message|Audio")
	void SendAudioSessionStopToConversation(UInworldPlayer* Player);

	/**
	 * Send a narration event for a character.
	 * @param Character The character for the narration event.
	 * @param Content The content of the narration.
	 */
	UFUNCTION(BlueprintCallable, Category = "Message|Narration")
	void SendNarrationEvent(UInworldCharacter* Character, const FString& Content);

	/**
	 * Send a trigger event to a character.
	 * @param Character The character to send the trigger event to.
	 * @param Name The name of the trigger.
	 * @param Params The parameters associated with the trigger.
	 */
	UFUNCTION(BlueprintCallable, Category = "Message|Trigger")
	void SendTrigger(UInworldCharacter* Character, const FString& Name, const TMap<FString, FString>& Params);
	/**
	 * Send a trigger event to a conversation for a player.
	 * @param Player The player to send the trigger event to.
	 * @param Name The name of the trigger.
	 * @param Params The parameters associated with the trigger.
	 */
	UFUNCTION(BlueprintCallable, Category = "Message|Trigger")
	void SendTriggerToConversation(UInworldPlayer* Player, const FString& Name, const TMap<FString, FString>& Params);

	/**
	 * Send a change scene event.
	 * @param SceneName The name of the scene to change to.
	 */
	UFUNCTION(BlueprintCallable, Category = "Message|Mutation")
	void SendChangeSceneEvent(const FString& SceneName);

	/**
	 * Cancel a response for a character.
	 * @param Character The character to cancel the response for.
	 * @param InteractionId The ID of the interaction to cancel the response for.
	 * @param UtteranceIds The IDs of the utterances to cancel.
	 */
	UFUNCTION(BlueprintCallable, Category = "Message|Mutation")
	void CancelResponse(UInworldCharacter* Character, const FString& InteractionId, const TArray<FString>& UtteranceIds);

	/**
	 * Delegate for handling pre-pause events in the Inworld session.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Connection")
	FOnInworldSessionPrePause OnPrePauseDelegate;
	FOnInworldSessionPrePauseNative& OnPrePause() { return OnPrePauseDelegateNative; }

	/**
	 * Delegate for handling pre-stop events in the Inworld session.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Connection")
	FOnInworldSessionPreStop OnPreStopDelegate;
	FOnInworldSessionPreStopNative& OnPreStop() { return OnPreStopDelegateNative; }

	/**
	 * Get the current connection state.
	 * @return The current connection state.
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

	/**
	 * Delegate for handling connection state changes.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Connection")
	FOnInworldConnectionStateChanged OnConnectionStateChangedDelegate;
	FOnInworldConnectionStateChangedNative& OnConnectionStateChanged() { return OnConnectionStateChangedDelegateNative; }

	/**
	 * Delegate for handling Inworld session loading events.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Connection")
	FOnInworldSessionLoaded OnLoadedDelegate;
	FOnInworldSessionLoadedNative& OnLoaded() { return OnLoadedDelegateNative; }

	/**
	 * Check if the Inworld session is loaded.
	 * @return True if the session is loaded, false otherwise.
	 */
	UFUNCTION(BlueprintPure, Category = "Connection")
	bool IsLoaded() const { return bIsLoaded; }

	/**
	 * Delegate for handling perceived latency events.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Connection")
	FOnInworldPerceivedLatency OnPerceivedLatencyDelegate;
	FOnInworldPerceivedLatencyNative& OnPerceivedLatency() { return OnPerceivedLatencyDelegateNative; }

private:
	void PossessAgents(const TArray<FInworldAgentInfo>& AgentInfos);
	void UnpossessAgents();

private:
	UPROPERTY()
	TObjectPtr<UInworldClient> Client;

	FString Workspace;

	UFUNCTION()
	void OnRep_IsLoaded();

	UPROPERTY(ReplicatedUsing = OnRep_IsLoaded)
	bool bIsLoaded;

	UFUNCTION()
	void OnRep_ConnectionState();

	UPROPERTY(ReplicatedUsing = OnRep_ConnectionState)
	EInworldConnectionState ConnectionState;

	FDelegateHandle OnClientPacketReceivedHandle;
	FDelegateHandle OnClientConnectionStateChangedHandle;
	FDelegateHandle OnClientPerceivedLatencyHandle;

	UPROPERTY(Replicated)
	TArray<UInworldCharacter*> RegisteredCharacters;
	UPROPERTY(Replicated)
	TArray<UInworldPlayer*> RegisteredPlayers;

	TMap<FString, UInworldCharacter*> BrainNameToCharacter;
	TMap<FString, UInworldCharacter*> AgentIdToCharacter;
	TMap<FString, FInworldAgentInfo> BrainNameToAgentInfo;
	TMap<FString, TArray<FString>> ConversationIdToAgentIds;
	TMap<FString, UInworldPlayer*> ConversationIdToPlayer;

	FOnInworldSessionPrePauseNative OnPrePauseDelegateNative;
	FOnInworldSessionPreStopNative OnPreStopDelegateNative;
	FOnInworldConnectionStateChangedNative OnConnectionStateChangedDelegateNative;
	FOnInworldSessionLoadedNative OnLoadedDelegateNative;
	FOnInworldPerceivedLatencyNative OnPerceivedLatencyDelegateNative;

	class FInworldSessionPacketVisitor : public TSharedFromThis<FInworldSessionPacketVisitor>, public InworldPacketVisitor
	{
	public:
		FInworldSessionPacketVisitor()
			: FInworldSessionPacketVisitor(nullptr)
		{}
		FInworldSessionPacketVisitor(class UInworldSession* InSession)
			: Session(InSession)
		{}
		virtual ~FInworldSessionPacketVisitor() = default;

		virtual void Visit(const FInworldControlEvent& Event) override;
		virtual void Visit(const FInworldConversationUpdateEvent& Event) override;
		virtual void Visit(const FInworldCurrentSceneStatusEvent& Event) override;

	private:
		UInworldSession* Session;
	};

	TSharedRef<FInworldSessionPacketVisitor> PacketVisitor;
};

UINTERFACE(MinimalAPI, BlueprintType)
class UInworldSessionOwnerInterface : public UInterface
{
	GENERATED_BODY()
};

class INWORLDAICLIENT_API IInworldSessionOwnerInterface
{
	GENERATED_BODY()

public:
	/**
	 * Get the Inworld Session.
	 * @return The Inworld Session.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inworld")
	UInworldSession* GetInworldSession() const;
};
