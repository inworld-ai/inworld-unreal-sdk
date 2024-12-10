/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "InworldEnums.h"
#include "InworldTypes.h"
#include "InworldPackets.h"

#if !UE_BUILD_SHIPPING
#include "HAL/IConsoleManager.h"
#endif

#include <memory>

#include "InworldClient.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInworldPacketReceived, const FInworldWrappedPacket&, WrappedPacket);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnInworldPacketReceivedCallback, const FInworldWrappedPacket&, WrappedPacket);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInworldPacketReceivedNative, const FInworldWrappedPacket& /*WrappedPacket*/);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInworldSessionPrePause);
DECLARE_MULTICAST_DELEGATE(FOnInworldSessionPrePauseNative);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInworldSessionPreStop);
DECLARE_MULTICAST_DELEGATE(FOnInworldSessionPreStopNative);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInworldConnectionStateChanged, EInworldConnectionState, ConnectionState);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnInworldConnectionStateChangedCallback, EInworldConnectionState, ConnectionState);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInworldConnectionStateChangedNative, EInworldConnectionState /*ConnectionState*/);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInworldPerceivedLatency, FString, InteractionId, int32, LatencyMs);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnInworldPerceivedLatencyCallback, FString, InteractionId, int32, LatencyMs);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnInworldPerceivedLatencyNative, FString /*InteractionId*/, int32 /*LatencyMs*/);

DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnInworldSessionSavedCallback, FInworldSave, Save, bool, bSuccess);

#ifdef INWORLD_WITH_NDK
namespace Inworld
{
	class Client;
}

class NDKClient
{
public:
	NDKClient() = default;
	virtual ~NDKClient() = default;

	virtual Inworld::Client& Get() const = 0;
};
#else

class IHttpRequest;
class IWebSocket;

class WSClient
{
public:
	WSClient() = default;
	virtual ~WSClient() = default;

	TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> TokenRequest;
	TSharedPtr<IWebSocket> ServiceSocket;
};
#endif

UCLASS(BlueprintType)
class INWORLDAICLIENT_API UInworldClient : public UObject
{
public:
	GENERATED_BODY()

	UInworldClient();
	~UInworldClient();
	FString RequestToken(const FString& WorkspaceOverride, const FInworldAuth& AuthOverride);
 	/**
	* Start a session from a scene.
	* @param Scene The scene to initialize.
	* @param PlayerProfile The player's profile.
	* @param CapabilitySet The capability set.
	* @param Metadata Additional metadata.
  * @param WorkspaceOverride The workspace to use instead of the project default.
	* @param AuthOverride The authentication to use instead of project default.
	*/
	UFUNCTION(BlueprintCallable, Category = "Session", meta = (AdvancedDisplay = "1", AutoCreateRefTerm = "PlayerProfile, CapabilitySet, Metadata, WorkspaceOverride, AuthOverride"))
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
  UFUNCTION(BlueprintCallable, Category = "Session", meta = (AdvancedDisplay = "1", AutoCreateRefTerm = "PlayerProfile, CapabilitySet, Metadata, WorkspaceOverride, AuthOverride"))
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
  UFUNCTION(BlueprintCallable, Category = "Session", meta = (AdvancedDisplay = "1", AutoCreateRefTerm = "PlayerProfile, CapabilitySet, Metadata, WorkspaceOverride, AuthOverride"))
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
	* Get the session ID.
	* @return The session ID.
	*/
	UFUNCTION(BlueprintPure, Category = "Session")
	FInworldToken GetSessionToken() const;

	/**
	* Load the Player Profile.
	* @param PlayerProfile The player profile.
	*/
	UFUNCTION(BlueprintCallable, Category = "Load|Configuration")
	void LoadPlayerProfile(const FInworldPlayerProfile& PlayerProfile);

	/**
	* Get the capabilities of the current session.
	* @return The capabilities.
	*/
	UFUNCTION(BlueprintPure, Category = "Session")
	FInworldCapabilitySet GetCapabilities() const;

	/**
	* Load the CapabilitySet.
	* @param CapabilitySet The capabilities.
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
	* Send interaction feedback.
	* @param InteractionId The ID of the interaction.
	* @param bIsLike Whether the interaction is liked.
	* @param Message The feedback message.
	*/
	UFUNCTION(BlueprintCallable, Category = "Session")
	void SendInteractionFeedback(const FString& InteractionId, bool bIsLike, const FString& Message);

	/**
	* Load a character with the specified ID.
	* @param Id The ID of the character to load.
	*/
	UFUNCTION(BlueprintCallable, Category = "Load|Character")
	void LoadCharacter(const FString& Id) { LoadCharacters({ Id }); }
	
	/**
	* Load multiple characters with the specified IDs.
	* @param Ids The IDs of the characters to load.
	*/
	UFUNCTION(BlueprintCallable, Category = "Load|Character")
	void LoadCharacters(const TArray<FString>& Ids);
	
	/**
	 * Unload a character with the specified ID.
	 * @param Id The ID of the character to unload.
	 */
	UFUNCTION(BlueprintCallable, Category = "Load|Character")
	void UnloadCharacter(const FString& Id) { UnloadCharacters({ Id }); }

	/**
	 * Unload multiple characters with the specified IDs.
	 * @param Ids The IDs of the characters to unload.
	 */
	UFUNCTION(BlueprintCallable, Category = "Load|Character")
	void UnloadCharacters(const TArray<FString>& Ids);

	/**
	 * Update a conversation with the specified parameters.
	 * @param ConversationId The ID of the conversation.
	 * @param AgentIds The IDs of the agents involved in the conversation.
	 * @param bIncludePlayer Whether to include the player in the conversation.
	 * @return The updated conversation.
	 */
	UFUNCTION(BlueprintCallable, Category = "Conversation")
	FString UpdateConversation(const FString& ConversationId, const TArray<FString>& AgentIds, bool bIncludePlayer);

	/**
	 * Send a text message to the specified agent.
	 * @param AgentId The ID of the agent.
	 * @param Text The text message to send.
	 * @return The wrapped packet containing the text message.
	 */
	UFUNCTION(BlueprintCallable, Category = "Message|Text")
	FInworldWrappedPacket SendTextMessage(const FString& AgentId, const FString& Text);

	/**
	 * Send a text message to a conversation.
	 * @param ConversationId The ID of the conversation.
	 * @param Text The text message to send.
	 * @return The wrapped packet containing the text message.
	 */
	UFUNCTION(BlueprintCallable, Category = "Message|Text")
	FInworldWrappedPacket SendTextMessageToConversation(const FString& ConversationId, const FString& Text);

	/**
	 * Initialize the speech processor with the specified mode and options.
	 * @param Mode The speech mode to initialize.
	 * @param SpeechOptions The speech options.
	 */
	UFUNCTION(BlueprintCallable, Category = "Message|Audio")
	void InitSpeechProcessor(EInworldPlayerSpeechMode Mode, const FInworldPlayerSpeechOptions& SpeechOptions);

	/**
	 * Destroy the speech processor.
	 */
	UFUNCTION(BlueprintCallable, Category = "Message|Audio")
	void DestroySpeechProcessor();

	/**
	 * Send a sound message to the specified agent.
	 * @param AgentId The ID of the agent.
	 * @param InputData The input sound data.
	 * @param OutputData The output sound data.
	 */
	UFUNCTION(BlueprintCallable, Category = "Message|Audio")
	void SendSoundMessage(const FString& AgentId, const TArray<uint8>& InputData, const TArray<uint8>& OutputData);
	/**
	 * Send a sound message to a conversation.
	 * @param ConversationId The ID of the conversation.
	 * @param InputData The input sound data.
	 * @param OutputData The output sound data.
	 */
	UFUNCTION(BlueprintCallable, Category = "Message|Audio")
	void SendSoundMessageToConversation(const FString& ConversationId, const TArray<uint8>& InputData, const TArray<uint8>& OutputData);

	/**
	 * Start an audio session for the specified agent.
	 * @param AgentId The ID of the agent.
	 * @param SessionOptions The audio session options.
	 */
	UFUNCTION(BlueprintCallable, Category = "Message|Audio")
	void SendAudioSessionStart(const FString& AgentId, FInworldAudioSessionOptions SessionOptions);
	/**
	 * Start an audio session for a conversation.
	 * @param ConversationId The ID of the conversation.
	 * @param SessionOptions The audio session options.
	 */
	UFUNCTION(BlueprintCallable, Category = "Message|Audio")
	void SendAudioSessionStartToConversation(const FString& ConversationId, FInworldAudioSessionOptions SessionOptions);

	/**
	 * Stop the audio session for the specified agent.
	 * @param AgentId The ID of the agent.
	 */
	UFUNCTION(BlueprintCallable, Category = "Message|Audio")
	void SendAudioSessionStop(const FString& AgentId);
	/**
	 * Stop the audio session for a conversation.
	 * @param ConversationId The ID of the conversation.
	 */
	UFUNCTION(BlueprintCallable, Category = "Message|Audio")
	void SendAudioSessionStopToConversation(const FString& ConversationId);

	/**
	 * Send a narration event for the specified agent.
	 * @param AgentId The ID of the agent.
	 * @param Content The narration content.
	 */
	UFUNCTION(BlueprintCallable, Category = "Message|Narration")
	void SendNarrationEvent(const FString& AgentId, const FString& Content);

	/**
	 * Send a trigger to the specified agent.
	 * @param AgentId The ID of the agent.
	 * @param Name The name of the trigger.
	 * @param Params The trigger parameters.
	 */
	UFUNCTION(BlueprintCallable, Category = "Message|Trigger")
	void SendTrigger(const FString& AgentId, const FString& Name, const TMap<FString, FString>& Params);
	/**
	 * Send a trigger to a conversation.
	 * @param ConversationId The ID of the conversation.
	 * @param Name The name of the trigger.
	 * @param Params The trigger parameters.
	 */
	UFUNCTION(BlueprintCallable, Category = "Message|Trigger")
	void SendTriggerToConversation(const FString& ConversationId, const FString& Name, const TMap<FString, FString>& Params);

	/**
	 * Send a change scene event with the specified scene name.
	 * @param SceneName The name of the scene to change to.
	 */
	UFUNCTION(BlueprintCallable, Category = "Message|Mutation")
	void SendChangeSceneEvent(const FString& SceneName);

	/**
	 * Cancel the response for the specified agent with the given interaction and utterance IDs.
	 * @param AgentId The ID of the agent.
	 * @param InteractionId The ID of the interaction.
	 * @param UtteranceIds The IDs of the utterances to cancel.
	 */
	UFUNCTION(BlueprintCallable, Category = "Message|Mutation")
	void CancelResponse(const FString& AgentId, const FString& InteractionId, const TArray<FString>& UtteranceIds);

	/**
	 * Create or update items in entities.
	 * @param Items The items to create or update.
	 * @param AddToEntities The entities to add the items to.
	 */
	UFUNCTION(BlueprintCallable, Category = "Message|Entity")
	void CreateOrUpdateItems(const TArray<FInworldEntityItem>& Items, const TArray<FString>& AddToEntities);

	/**
	 * Remove items from entities.
	 * @param ItemIds The IDs of the items to remove.
	 */
	UFUNCTION(BlueprintCallable, Category = "Message|Entity")
	void RemoveItems(const TArray<FString>& ItemIds);

	/**
	 * Add items to entities.
	 * @param ItemIds The IDs of the items to add.
	 * @param EntityNames The names of the entities to add the items to.
	 */
	UFUNCTION(BlueprintCallable, Category = "Message|Entity")
	void AddItemsInEntities(const TArray<FString>& ItemIds, const TArray<FString>& EntityNames);

	/**
	 * Remove items from entities.
	 * @param ItemIds The IDs of the items to remove.
	 * @param EntityNames The names of the entities to remove the items from.
	 */
	UFUNCTION(BlueprintCallable, Category = "Message|Entity")
	void RemoveItemsInEntities(const TArray<FString>& ItemIds, const TArray<FString>& EntityNames);

	/**
	 * Replace items in entities.
	 * @param ItemIds The IDs of the items to replace.
	 * @param EntityNames The names of the entities to replace the items in.
	 */
	UFUNCTION(BlueprintCallable, Category = "Message|Entity")
	void ReplaceItemsInEntities(const TArray<FString>& ItemIds, const TArray<FString>& EntityNames);

	/**
	 * Event dispatcher for when a packet is received.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Packet")
	FOnInworldPacketReceived OnPacketReceivedDelegate;
	FOnInworldPacketReceivedNative& OnPacketReceived() { return OnPacketReceivedDelegateNative; }

	/**
	 * Event dispatcher for when the session is about to pause.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Connection")
	FOnInworldSessionPrePause OnPrePauseDelegate;
	FOnInworldSessionPrePauseNative& OnPrePause() { return OnPrePauseDelegateNative; }

	/**
	 * Event dispatcher for when the session is about to stop.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Connection")
	FOnInworldSessionPreStop OnPreStopDelegate;
	FOnInworldSessionPreStopNative& OnPreStop() { return OnPreStopDelegateNative; }

	/**
	 * Get the current connection state.
	 * @return The connection state.
	 */
	UFUNCTION(BlueprintPure, Category = "Connection")
	EInworldConnectionState GetConnectionState() const;
	/**
	 * Get the connection error details.
	 * @param OutErrorMessage The error message.
	 * @param OutErrorCode The error code.
	 * @param OutErrorDetails Additional error details.
	 */
	UFUNCTION(BlueprintPure, Category = "Connection")
	void GetConnectionError(FString& OutErrorMessage, int32& OutErrorCode, FInworldConnectionErrorDetails& OutErrorDetails) const;

	/**
	 * Event dispatcher for when the connection state changes.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Connection")
	FOnInworldConnectionStateChanged OnConnectionStateChangedDelegate;
	FOnInworldConnectionStateChangedNative& OnConnectionStateChanged() { return OnConnectionStateChangedDelegateNative; }

	/**
	 * Event dispatcher for perceived latency changes.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Connection")
	FOnInworldPerceivedLatency OnPerceivedLatencyDelegate;
	FOnInworldPerceivedLatencyNative& OnPerceivedLatency() { return OnPerceivedLatencyDelegateNative; }

private:
	FOnInworldSessionPrePauseNative OnPrePauseDelegateNative;
	FOnInworldSessionPreStopNative OnPreStopDelegateNative;
	FOnInworldPacketReceivedNative OnPacketReceivedDelegateNative;
	FOnInworldConnectionStateChangedNative OnConnectionStateChangedDelegateNative;
	FOnInworldPerceivedLatencyNative OnPerceivedLatencyDelegateNative;

	bool bIsBeingDestroyed = false;

#ifdef INWORLD_WITH_NDK
#if !UE_BUILD_SHIPPING
	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnAudioDumperCVarChanged, bool /*Enabled*/, FString /*Path*/);
	static FOnAudioDumperCVarChanged OnAudioDumperCVarChanged;
	FDelegateHandle OnAudioDumperCVarChangedHandle;
	static FAutoConsoleVariableSink CVarSink;
	static void OnCVarsChanged();
#endif

	TUniquePtr<NDKClient> Client;
#else
	TUniquePtr<WSClient> Client;
#endif
};