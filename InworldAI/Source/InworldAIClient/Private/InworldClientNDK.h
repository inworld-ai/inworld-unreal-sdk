/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#ifdef INWORLD_WITH_NDK

#include "InworldClient.h"
#include "InworldEnums.h"
#include "InworldTypes.h"
#include "InworldPackets.h"

#if !UE_BUILD_SHIPPING
#include "HAL/IConsoleManager.h"
#endif

#include <memory>

namespace Inworld
{
    class Client;
}

class InworldClientNDK : public IInworldClientImplInterface
{
public:
    // Constructor and Destructor
    InworldClientNDK();
    virtual ~InworldClientNDK();

    // Session Management
    virtual void StartSessionFromScene(const FInworldScene& Scene, const FInworldPlayerProfile& PlayerProfile, const FInworldCapabilitySet& CapabilitySet, const TMap<FString, FString>& Metadata, const FString& WorkspaceOverride, const FInworldAuth& AuthOverride) override;
    virtual void StartSessionFromSave(const FInworldSave& Save, const FInworldPlayerProfile& PlayerProfile, const FInworldCapabilitySet& CapabilitySet, const TMap<FString, FString>& Metadata, const FString& WorkspaceOverride, const FInworldAuth& AuthOverride) override;
    virtual void StartSessionFromToken(const FInworldToken& Token, const FInworldPlayerProfile& PlayerProfile, const FInworldCapabilitySet& CapabilitySet, const TMap<FString, FString>& Metadata, const FString& WorkspaceOverride, const FInworldAuth& AuthOverride) override;
    virtual void StopSession() override;
    virtual void PauseSession() override;
    virtual void ResumeSession() override;

    // Session Information
    virtual FInworldToken GetSessionToken() const override;

    // Player Profile and Capabilities
    virtual void LoadPlayerProfile(const FInworldPlayerProfile& PlayerProfile) override;
    virtual FInworldCapabilitySet GetCapabilities() const override;
    virtual void LoadCapabilities(const FInworldCapabilitySet& CapabilitySet) override;

    // Session Persistence
    virtual void SaveSession(FOnInworldSessionSavedCallback Callback) override;

    // Interaction Feedback
    virtual void SendInteractionFeedback(const FString& InteractionId, bool bIsLike, const FString& Message) override;

    // Character Management
    virtual void LoadCharacter(const FString& Id) override;
    virtual void LoadCharacters(const TArray<FString>& Ids) override;
    virtual void UnloadCharacter(const FString& Id) override;
    virtual void UnloadCharacters(const TArray<FString>& Ids) override;

    // Conversation Management
    virtual FString UpdateConversation(const FString& ConversationId, const TArray<FString>& AgentIds, bool bIncludePlayer) override;

    // Messaging
    virtual FInworldWrappedPacket SendTextMessage(const FString& AgentId, const FString& Text) override;
    virtual FInworldWrappedPacket SendTextMessageToConversation(const FString& ConversationId, const FString& Text) override;

    // Speech Processing
    virtual void InitSpeechProcessor(EInworldPlayerSpeechMode Mode, const FInworldPlayerSpeechOptions& SpeechOptions) override;
    virtual void DestroySpeechProcessor() override;

    // Audio Messaging
    virtual void SendSoundMessage(const FString& AgentId, const TArray<uint8>& InputData, const TArray<uint8>& OutputData) override;
    virtual void SendSoundMessageToConversation(const FString& ConversationId, const TArray<uint8>& InputData, const TArray<uint8>& OutputData) override;

    // Audio Session Management
    virtual void SendAudioSessionStart(const FString& AgentId, FInworldAudioSessionOptions SessionOptions) override;
    virtual void SendAudioSessionStartToConversation(const FString& ConversationId, FInworldAudioSessionOptions SessionOptions) override;
    virtual void SendAudioSessionStop(const FString& AgentId) override;
    virtual void SendAudioSessionStopToConversation(const FString& ConversationId) override;

    // Narration and Trigger Events
    virtual void SendNarrationEvent(const FString& AgentId, const FString& Content) override;
    virtual void SendTrigger(const FString& AgentId, const FString& Name, const TMap<FString, FString>& Params) override;
    virtual void SendTriggerToConversation(const FString& ConversationId, const FString& Name, const TMap<FString, FString>& Params) override;

    // Scene and Mutation Events
    virtual void SendChangeSceneEvent(const FString& SceneName) override;
    virtual void CancelResponse(const FString& AgentId, const FString& InteractionId, const TArray<FString>& UtteranceIds) override;

    // Entity Management
    virtual void CreateOrUpdateItems(const TArray<FInworldEntityItem>& Items, const TArray<FString>& AddToEntities) override;
    virtual void RemoveItems(const TArray<FString>& ItemIds) override;
    virtual void AddItemsInEntities(const TArray<FString>& ItemIds, const TArray<FString>& EntityNames) override;
    virtual void RemoveItemsInEntities(const TArray<FString>& ItemIds, const TArray<FString>& EntityNames) override;
    virtual void ReplaceItemsInEntities(const TArray<FString>& ItemIds, const TArray<FString>& EntityNames) override;

    // Connection Information
    virtual EInworldConnectionState GetConnectionState() const override;
    virtual void GetConnectionError(FString& OutErrorMessage, int32& OutErrorCode, FInworldConnectionErrorDetails& OutErrorDetails) const override;

    virtual FOnInworldPacketReceivedNative& OnPacketReceived() override { return OnPacketReceivedDelegateNative; };
    virtual FOnInworldConnectionStateChangedNative& OnConnectionStateChanged() override { return OnConnectionStateChangedDelegateNative; }
    virtual FOnInworldPerceivedLatencyNative& OnPerceivedLatency() override { return OnPerceivedLatencyDelegateNative; }

private:
    std::unique_ptr<Inworld::Client> NDKClient;

    FOnInworldPacketReceivedNative OnPacketReceivedDelegateNative;
    FOnInworldConnectionStateChangedNative OnConnectionStateChangedDelegateNative;
    FOnInworldPerceivedLatencyNative OnPerceivedLatencyDelegateNative;

#if !UE_BUILD_SHIPPING
    DECLARE_MULTICAST_DELEGATE_TwoParams(FOnAudioDumperCVarChanged, bool /*Enabled*/, FString /*Path*/);
    static FOnAudioDumperCVarChanged OnAudioDumperCVarChanged;
    FDelegateHandle OnAudioDumperCVarChangedHandle;
    static FAutoConsoleVariableSink CVarSink;
    static void OnCVarsChanged();
#endif
};

#endif
