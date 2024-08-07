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
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInworldVAD, UInworldPlayer*, Player, bool, bVoiceDetected);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInworldSessionLoaded, bool, bLoaded);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInworldSessionLoadedNative, bool /*bLoaded*/);

UCLASS(BlueprintType)
class INWORLDAIINTEGRATION_API UInworldSession : public UObject
{
	GENERATED_BODY()
public:
	UInworldSession();
	virtual ~UInworldSession();

	// UObject
	virtual UWorld* GetWorld() const override { return GetTypedOuter<AActor>()->GetWorld(); }
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual int32 GetFunctionCallspace(UFunction* Function, FFrame* Stack) override;
	virtual bool CallRemoteFunction(UFunction* Function, void* Parms, struct FOutParmRec* OutParms, FFrame* Stack) override;
	// ~UObject

public:
	UFUNCTION(BlueprintCallable, Category = "Client")
	void Init();
	UFUNCTION(BlueprintCallable, Category = "Client")
	void Destroy();

	UFUNCTION(BlueprintPure, Category = "Client")
	UInworldClient* GetClient() const { return Client; }

	UFUNCTION()
	void HandlePacket(const FInworldWrappedPacket& WrappedPacket);

	UFUNCTION(BlueprintCallable, Category = "Register")
	void RegisterCharacter(UInworldCharacter* Character);
	UFUNCTION(BlueprintCallable, Category = "Register")
	void UnregisterCharacter(UInworldCharacter* Character);

	UFUNCTION(BlueprintPure, Category = "Register")
	const TArray<UInworldCharacter*>& GetRegisteredCharacters() const { return RegisteredCharacters; }

	UFUNCTION(BlueprintCallable, Category = "Register")
	void RegisterPlayer(UInworldPlayer* Player);
	UFUNCTION(BlueprintCallable, Category = "Register")
	void UnregisterPlayer(UInworldPlayer* Player);

	UFUNCTION(BlueprintPure, Category = "Register")
	const TArray<UInworldPlayer*>& GetRegisteredPlayers() const { return RegisteredPlayers; }

	UFUNCTION(BlueprintCallable, Category = "Session", meta = (AdvancedDisplay = "4", AutoCreateRefTerm = "PlayerProfile, Auth, Save, SessionToken, CapabilitySet"))
	void StartSession(const FInworldPlayerProfile& PlayerProfile, const FInworldAuth& Auth, const FString& SceneId, const FInworldSave& Save,
		const FInworldSessionToken& SessionToken, const FInworldCapabilitySet& CapabilitySet, const FInworldPlayerSpeechOptions& SpeechOptions, const TMap<FString, FString>& Metadata);
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

	UFUNCTION(BlueprintCallable, Category = "Load|Character")
	void LoadCharacter(UInworldCharacter* Character) { LoadCharacters({ Character }); }
	UFUNCTION(BlueprintCallable, Category = "Load|Character")
	void LoadCharacters(const TArray<UInworldCharacter*>& Characters);
	UFUNCTION(BlueprintCallable, Category = "Load|Character")
	void UnloadCharacter(UInworldCharacter* Character) { UnloadCharacters({ Character }); }
	UFUNCTION(BlueprintCallable, Category = "Load|Character")
	void UnloadCharacters(const TArray<UInworldCharacter*>& Characters);

	UFUNCTION(BlueprintCallable, Category = "Conversation")
	FString UpdateConversation(UInworldPlayer* Player);

	UFUNCTION(BlueprintCallable, Category = "Message|Text")
	void SendTextMessage(UInworldCharacter* Character, const FString& Message);
	UFUNCTION(BlueprintCallable, Category = "Message|Text")
	void SendTextMessageToConversation(UInworldPlayer* Player, const FString& Message);

	UFUNCTION(BlueprintCallable, Category = "Message|Audio")
	void SendSoundMessage(UInworldCharacter* Character, const TArray<uint8>& InputData, const TArray<uint8>& OutputData);
	UFUNCTION(BlueprintCallable, Category = "Message|Audio")
	void SendSoundMessageToConversation(UInworldPlayer* Player, const TArray<uint8>& InputData, const TArray<uint8>& OutputData);

	UFUNCTION(BlueprintCallable, Category = "Message|Audio")
	void SendAudioSessionStart(UInworldCharacter* Character, UInworldPlayer* Player, FInworldAudioSessionOptions SessionOptions);
	UFUNCTION(BlueprintCallable, Category = "Message|Audio")
	void SendAudioSessionStartToConversation(UInworldPlayer* Player, FInworldAudioSessionOptions SessionOptions);

	UFUNCTION(BlueprintCallable, Category = "Message|Audio")
	void SendAudioSessionStop(UInworldCharacter* Character);
	UFUNCTION(BlueprintCallable, Category = "Message|Audio")
	void SendAudioSessionStopToConversation(UInworldPlayer* Player);

	UFUNCTION(BlueprintCallable, Category = "Message|Narration")
	void SendNarrationEvent(UInworldCharacter* Character, const FString& Content);

	UFUNCTION(BlueprintCallable, Category = "Message|Trigger")
	void SendTrigger(UInworldCharacter* Character, const FString& Name, const TMap<FString, FString>& Params);
	UFUNCTION(BlueprintCallable, Category = "Message|Trigger")
	void SendTriggerToConversation(UInworldPlayer* Player, const FString& Name, const TMap<FString, FString>& Params);

	UFUNCTION(BlueprintCallable, Category = "Message|Mutation")
	void SendChangeSceneEvent(const FString& SceneName);

	UFUNCTION(BlueprintCallable, Category = "Message|Mutation")
	void CancelResponse(UInworldCharacter* Character, const FString& InteractionId, const TArray<FString>& UtteranceIds);

	UFUNCTION(BlueprintPure, Category = "Connection")
	EInworldConnectionState GetConnectionState() const;
	UFUNCTION(BlueprintPure, Category = "Connection")
	void GetConnectionError(FString& OutErrorMessage, int32& OutErrorCode, FInworldConnectionErrorDetails& OutErrorDetails) const;

	UPROPERTY(BlueprintAssignable, Category = "Connection")
	FOnInworldConnectionStateChanged OnConnectionStateChangedDelegate;
	FOnInworldConnectionStateChangedNative& OnConnectionStateChanged() { return OnConnectionStateChangedDelegateNative; }

	UPROPERTY(BlueprintAssignable, Category = "Connection")
	FOnInworldSessionLoaded OnLoadedDelegate;
	FOnInworldSessionLoadedNative& OnLoaded() { return OnLoadedDelegateNative; }

	UFUNCTION(BlueprintPure, Category = "Connection")
	bool IsLoaded() const { return bIsLoaded; }

	UPROPERTY(BlueprintAssignable, Category = "Connection")
	FOnInworldPerceivedLatency OnPerceivedLatencyDelegate;
	FOnInworldPerceivedLatencyNative& OnPerceivedLatency() { return OnPerceivedLatencyDelegateNative; }

	UPROPERTY(BlueprintAssignable, Category = "VAD")
	FOnInworldVAD OnVADDelegate;
	FOnInworldVADNative& OnVAD() { return OnVADDelegateNative; }

private:
	void PossessAgents(const TArray<FInworldAgentInfo>& AgentInfos);
	void UnpossessAgents();

private:
	UPROPERTY()
	UInworldClient* Client;

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
	FDelegateHandle OnVADHandle;

	UPROPERTY(Replicated)
	TArray<UInworldCharacter*> RegisteredCharacters;
	UPROPERTY(Replicated)
	TArray<UInworldPlayer*> RegisteredPlayers;

	TMap<FString, UInworldCharacter*> BrainNameToCharacter;
	TMap<FString, UInworldCharacter*> AgentIdToCharacter;
	TMap<FString, FInworldAgentInfo> BrainNameToAgentInfo;
	TMap<FString, TArray<FString>> ConversationIdToAgentIds;
	TMap<FString, UInworldPlayer*> ConversationIdToPlayer;

	FOnInworldConnectionStateChangedNative OnConnectionStateChangedDelegateNative;
	FOnInworldSessionLoadedNative OnLoadedDelegateNative;
	FOnInworldPerceivedLatencyNative OnPerceivedLatencyDelegateNative;
	FOnInworldVADNative OnVADDelegateNative;

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

class INWORLDAIINTEGRATION_API IInworldSessionOwnerInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inworld")
	UInworldSession* GetInworldSession() const;
};
