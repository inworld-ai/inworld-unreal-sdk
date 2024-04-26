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
#include "InworldSession.generated.h"

class UInworldPlayer;
class UInworldCharacter;
class UInworldClient;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInworldTextEvent, const FInworldTextEvent&, TextEvent);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInworldTextEventNative, const FInworldTextEvent& /*TextEvent*/);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInworldAudioEvent, const FInworldAudioDataEvent&, AudioEvent);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInworldAudioEventNative, const FInworldAudioDataEvent& /*AudioEvent*/);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInworldSilenceEvent, const FInworldSilenceEvent&, SilenceEvent);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInworldSilenceEventNative, const FInworldSilenceEvent& /*SilenceEvent*/);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInworldControlEvent, const FInworldControlEvent&, ControlEvent);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInworldControlEventNative, const FInworldControlEvent& /*ControlEvent*/);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInworldEmotionEvent, const FInworldEmotionEvent&, EmotionEvent);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInworldEmotionEventNative, const FInworldEmotionEvent& /*EmotionEvent*/);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInworldCustomEvent, const FInworldCustomEvent&, CustomEvent);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInworldCustomEventNative, const FInworldCustomEvent& /*CustomEvent*/);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInworldSessionLoaded, bool, bLoaded);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInworldSessionLoadedNative, bool /*bLoaded*/);

UCLASS(BlueprintType)
class INWORLDAIINTEGRATION_API UInworldSession : public UObject
{
	GENERATED_BODY()
public:
	// UObject
	virtual UWorld* GetWorld() const override { return GetTypedOuter<AActor>()->GetWorld(); }
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual int32 GetFunctionCallspace(UFunction* Function, FFrame* Stack) override;
	virtual bool CallRemoteFunction(UFunction* Function, void* Parms, struct FOutParmRec* OutParms, FFrame* Stack) override;
	// ~UObject

public:
	UInworldSession();
	virtual ~UInworldSession();

	UFUNCTION(BlueprintCallable, Category = "Client")
	void Init();
	UFUNCTION(BlueprintCallable, Category = "Client")
	void Destroy();

	UFUNCTION(BlueprintCallable, Category = "Inworld|Register")
	void RegisterCharacter(UInworldCharacter* Character);
	UFUNCTION(BlueprintCallable, Category = "Inworld|Register")
	void UnregisterCharacter(UInworldCharacter* Character);

	UFUNCTION(BlueprintPure, Category = "Inworld|Register")
	const TArray<UInworldCharacter*>& GetRegisteredCharacters() const { return RegisteredCharacters; }

	UFUNCTION(BlueprintCallable, Category = "Inworld|Session", meta = (AdvancedDisplay = "4", AutoCreateRefTerm = "PlayerProfile, Auth, Save, SessionToken, CapabilitySet"))
	void StartSession(const FString& SceneId, const FInworldPlayerProfile& PlayerProfile, const FInworldAuth& Auth, const FInworldSave& Save, const FInworldSessionToken& SessionToken, const FInworldCapabilitySet& CapabilitySet);
	UFUNCTION(BlueprintCallable, Category = "Inworld|Session")
	void StopSession();
	UFUNCTION(BlueprintCallable, Category = "Inworld|Session")
	void PauseSession() { InworldClient->PauseSession(); }
	UFUNCTION(BlueprintCallable, Category = "Inworld|Session")
	void ResumeSession() { InworldClient->ResumeSession(); }

	UFUNCTION(BlueprintPure, Category = "Inworld|Session")
	FString GetSessionId() const { return InworldClient->GetSessionId(); }

	UFUNCTION(BlueprintCallable, Category = "Inworld|Session")
	void SaveSession(FOnInworldSessionSavedCallback Callback) { InworldClient->SaveSession(Callback); }

	UFUNCTION(BlueprintCallable, Category = "Inworld|Load|Character")
	void LoadCharacter(UInworldCharacter* Character) { LoadCharacters({ Character }); }
	UFUNCTION(BlueprintCallable, Category = "Inworld|Load|Character")
	void LoadCharacters(const TArray<UInworldCharacter*>& Characters);
	UFUNCTION(BlueprintCallable, Category = "Inworld|Load|Character")
	void UnloadCharacter(UInworldCharacter* Character) { UnloadCharacters({ Character }); }
	UFUNCTION(BlueprintCallable, Category = "Inworld|Load|Character")
	void UnloadCharacters(const TArray<UInworldCharacter*>& Characters);

	UFUNCTION(BlueprintCallable, Category = "Inworld|Load")
	void LoadSavedState(const FInworldSave& Save) { InworldClient->LoadSavedState(Save); }
	UFUNCTION(BlueprintCallable, Category = "Inworld|Load")
	void LoadCapabilities(const FInworldCapabilitySet& CapabilitySet) { InworldClient->LoadCapabilities(CapabilitySet); }
	UFUNCTION(BlueprintCallable, Category = "Inworld|Load")
	void LoadPlayerProfile(const FInworldPlayerProfile& PlayerProfile) { InworldClient->LoadPlayerProfile(PlayerProfile); }

	UFUNCTION(BlueprintCallable, Category = "Message|Text")
	void SendTextMessage(UInworldCharacter* Character, const FString& Message) { BroadcastTextMessage({Character}, Message); }
	UFUNCTION(BlueprintCallable, Category = "Message|Text")
	void BroadcastTextMessage(const TArray<UInworldCharacter*>& Characters, const FString& Message);

	UFUNCTION(BlueprintCallable, Category = "Message|Audio")
	void SendSoundMessage(UInworldCharacter* Character, const TArray<uint8>& InputData, const TArray<uint8>& OutputData) { BroadcastSoundMessage({ Character }, InputData, OutputData); }
	UFUNCTION(BlueprintCallable, Category = "Message|Audio")
	void BroadcastSoundMessage(const TArray<UInworldCharacter*>& Characters, const TArray<uint8>& InputData, const TArray<uint8>& OutputData);
	UFUNCTION(BlueprintCallable, Category = "Message|Audio")
	void SendAudioSessionStart(UInworldCharacter* Character) { BroadcastAudioSessionStart({ Character }); }
	UFUNCTION(BlueprintCallable, Category = "Message|Audio")
	void BroadcastAudioSessionStart(const TArray<UInworldCharacter*>& Characters);
	UFUNCTION(BlueprintCallable, Category = "Message|Audio")
	void SendAudioSessionStop(UInworldCharacter* Character) { BroadcastAudioSessionStop({ Character }); }
	UFUNCTION(BlueprintCallable, Category = "Message|Audio")
	void BroadcastAudioSessionStop(const TArray<UInworldCharacter*>& Characters);

	UFUNCTION(BlueprintCallable, Category = "Message|Narration")
	void SendNarrationEvent(UInworldCharacter* Character, const FString& Content);

	UFUNCTION(BlueprintCallable, Category = "Message|Trigger")
	void SendTrigger(UInworldCharacter* Character, const FString& Name, const TMap<FString, FString>& Params) { BroadcastTrigger({ Character }, Name, Params); }
	UFUNCTION(BlueprintCallable, Category = "Message|Trigger")
	void BroadcastTrigger(const TArray<UInworldCharacter*>& Characters, const FString& Name, const TMap<FString, FString>& Params);

	UFUNCTION(BlueprintCallable, Category = "Message|Mutation")
	void SendChangeSceneEvent(const FString& SceneName);

	UFUNCTION(BlueprintCallable, Category = "Message|Mutation")
	void CancelResponse(UInworldCharacter* Character, const FString& InteractionId, const TArray<FString>& UtteranceIds);

	UPROPERTY(BlueprintAssignable, Category = "Event")
	FOnInworldTextEvent OnInworldTextEventDelegate;
	FOnInworldTextEventNative& OnInworldTextEvent() { return OnInworldTextEventDelegateNative; }
	UPROPERTY(BlueprintAssignable, Category = "Event")
	FOnInworldAudioEvent OnInworldAudioEventDelegate;
	FOnInworldAudioEventNative& OnInworldAudioEvent() { return OnInworldAudioEventDelegateNative; }
	UPROPERTY(BlueprintAssignable, Category = "Event")
	FOnInworldSilenceEvent OnInworldSilenceEventDelegate;
	FOnInworldSilenceEventNative& OnInworldSilenceEvent() { return OnInworldSilenceEventDelegateNative; }
	UPROPERTY(BlueprintAssignable, Category = "Event")
	FOnInworldControlEvent OnInworldControlEventDelegate;
	FOnInworldControlEventNative& OnInworldControlEvent() { return OnInworldControlEventDelegateNative; }
	UPROPERTY(BlueprintAssignable, Category = "Event")
	FOnInworldEmotionEvent OnInworldEmotionEventDelegate;
	FOnInworldEmotionEventNative& OnInworldEmotionEvent() { return OnInworldEmotionEventDelegateNative; }
	UPROPERTY(BlueprintAssignable, Category = "Event")
	FOnInworldCustomEvent OnInworldCustomEventDelegate;
	FOnInworldCustomEventNative& OnInworldCustomEvent() { return OnInworldCustomEventDelegateNative; }

	UFUNCTION(BlueprintPure, Category = "Connection")
	EInworldConnectionState GetConnectionState() const { return InworldClient->GetConnectionState(); }
	UFUNCTION(BlueprintPure, Category = "Connection")
	void GetConnectionError(FString& OutErrorMessage, int32& OutErrorCode) const { InworldClient->GetConnectionError(OutErrorMessage, OutErrorCode); }

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

private:
	void PossessAgents(const TArray<FInworldAgentInfo>& AgentInfos);
	void UnpossessAgents();

private:
	UPROPERTY()
	UInworldClient* InworldClient;

	UFUNCTION()
	void OnRep_IsLoaded();

	UPROPERTY(ReplicatedUsing = OnRep_IsLoaded)
	bool bIsLoaded;

	FDelegateHandle OnClientPacketReceivedHandle;
	FDelegateHandle OnClientConnectionStateChangedHandle;
	FDelegateHandle OnClientPerceivedLatencyHandle;

	UPROPERTY(Replicated)
	TArray<UInworldCharacter*> RegisteredCharacters;
	TMap<FString, UInworldCharacter*> BrainNameToCharacter;
	TMap<FString, UInworldCharacter*> AgentIdToCharacter;
	TMap<FString, FInworldAgentInfo> BrainNameToAgentInfo;

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

		virtual void Visit(const FInworldTextEvent& Event) override;
		virtual void Visit(const FInworldAudioDataEvent& Event) override;
		virtual void Visit(const FInworldSilenceEvent& Event) override;
		virtual void Visit(const FInworldControlEvent& Event) override;
		virtual void Visit(const FInworldEmotionEvent& Event) override;
		virtual void Visit(const FInworldCustomEvent& Event) override;
		virtual void Visit(const FInworldLoadCharactersEvent& Event) override;
		virtual void Visit(const FInworldChangeSceneEvent& Event) override;

	private:
		UInworldSession* Session;
	};

	TSharedRef<FInworldSessionPacketVisitor> PacketVisitor;

	FOnInworldTextEventNative OnInworldTextEventDelegateNative;
	FOnInworldAudioEventNative OnInworldAudioEventDelegateNative;
	FOnInworldSilenceEventNative OnInworldSilenceEventDelegateNative;
	FOnInworldControlEventNative OnInworldControlEventDelegateNative;
	FOnInworldEmotionEventNative OnInworldEmotionEventDelegateNative;
	FOnInworldCustomEventNative OnInworldCustomEventDelegateNative;

private:
	float RetryConnectionIntervalTime = 0.25f;
	float MaxRetryConnectionTime = 5.0f;
	float CurrentRetryConnectionTime = 1.0f;

	FTimerHandle RetryConnectionTimerHandle;

	// Temp: Hack until deprecated functions are removed
	friend class UInworldApiSubsystem;
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
