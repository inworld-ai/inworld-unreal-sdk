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
#include "InworldTypes.h"
#include "InworldSession.h"
#include "InworldCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInworldCharacterPossessed, bool, bPossessed);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInworldCharacterPossessedNative, bool /*bPossessed*/);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInworldCharacterTargetPlayerChanged);
DECLARE_MULTICAST_DELEGATE(FOnInworldCharacterTargetPlayerChangedNative);

UCLASS(BlueprintType)
class INWORLDAIINTEGRATION_API UInworldCharacter : public UObject
{
	GENERATED_BODY()
public:
	UInworldCharacter();
	virtual ~UInworldCharacter();

	// UObject
	virtual UWorld* GetWorld() const override { return GetTypedOuter<AActor>()->GetWorld(); }
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual int32 GetFunctionCallspace(UFunction* Function, FFrame* Stack) override;
	virtual bool CallRemoteFunction(UFunction* Function, void* Parms, struct FOutParmRec* OutParms, FFrame* Stack) override;
	// ~UObject

public:
	UFUNCTION()
	void HandlePacket(const FInworldWrappedPacket& WrappedPacket);

	UFUNCTION(BlueprintCallable, Category="Session")
	void SetSession(UInworldSession* InSession);
	UFUNCTION(BlueprintPure, Category = "Session")
	UInworldSession* GetSession() const { return Session; }

	UFUNCTION(BlueprintCallable, Category = "Message|Text")
	void SendTextMessage(const FString& Text);
	UFUNCTION(BlueprintCallable, Category = "Message|Trigger")
	void SendTrigger(const FString& Name, const TMap<FString, FString>& Params);
	UFUNCTION(BlueprintCallable, Category = "Message|Narration")
	void SendNarrationEvent(const FString& Content);
	UFUNCTION(BlueprintCallable, Category = "Message|Audio")
	void SendAudioSessionStart(EInworldMicrophoneMode MicrophoneMode = EInworldMicrophoneMode::OPEN_MIC);
	UFUNCTION(BlueprintCallable, Category = "Message|Audio")
	void SendAudioSessionStop();
	UFUNCTION(BlueprintCallable, Category = "Message|Audio")
	void SendSoundMessage(const TArray<uint8>& Input, const TArray<uint8>& Output);
	UFUNCTION(BlueprintCallable, Category = "Message|Mutation")
	void CancelResponse(const FString& InteractionId, const TArray<FString>& UtteranceIds);

public:
	UFUNCTION(BlueprintCallable, Category = "Character")
	TScriptInterface<IInworldCharacterOwnerInterface> GetInworldCharacterOwner();

	UFUNCTION(BlueprintCallable, Category = "Possession")
	void SetBrainName(const FString& BrainName);

	UFUNCTION(BlueprintPure, Category = "Possession")
	bool IsPossessed() const { return !AgentInfo.AgentId.IsEmpty(); }
	UFUNCTION(BlueprintCallable, Category = "Possession")
	void Possess(const FInworldAgentInfo& InAgentInfo);
	UFUNCTION(BlueprintCallable, Category = "Possession")
	void Unpossess();

	UFUNCTION(BlueprintPure, Category = "Possession")
	const FInworldAgentInfo& GetAgentInfo() const { return AgentInfo; }

	UPROPERTY(BlueprintAssignable, Category = "Possession")
	FOnInworldCharacterPossessed OnPossessedDelegate;
	FOnInworldCharacterPossessedNative& OnPossessed() { return OnPossessedDelegateNative; }

	UFUNCTION(BlueprintCallable, Category = "Target")
	void SetTargetPlayer(UInworldPlayer* Player);
	UFUNCTION(BlueprintCallable, Category = "Target")
	void ClearTargetPlayer();

	UFUNCTION(BlueprintPure, Category = "Target")
	UInworldPlayer* GetTargetPlayer() const { return TargetPlayer; }

	UPROPERTY(BlueprintAssignable, Category = "Target")
	FOnInworldCharacterTargetPlayerChanged OnTargetPlayerChangedDelegate;
	FOnInworldCharacterTargetPlayerChangedNative& OnTargetPlayerChanged() { return OnTargetPlayerChangedDelegateNative; }

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

private:
	UPROPERTY(Replicated)
	UInworldSession* Session;

	UPROPERTY(Replicated)
	FInworldAgentInfo AgentInfo;
	FOnInworldCharacterPossessedNative OnPossessedDelegateNative;

	UFUNCTION()
	void OnRep_TargetPlayer();

	UPROPERTY(ReplicatedUsing=OnRep_TargetPlayer)
	UInworldPlayer* TargetPlayer;
	FOnInworldCharacterTargetPlayerChangedNative OnTargetPlayerChangedDelegateNative;

	FOnInworldTextEventNative OnInworldTextEventDelegateNative;
	FOnInworldAudioEventNative OnInworldAudioEventDelegateNative;
	FOnInworldSilenceEventNative OnInworldSilenceEventDelegateNative;
	FOnInworldControlEventNative OnInworldControlEventDelegateNative;
	FOnInworldEmotionEventNative OnInworldEmotionEventDelegateNative;
	FOnInworldCustomEventNative OnInworldCustomEventDelegateNative;

	class FInworldCharacterPacketVisitor : public TSharedFromThis<FInworldCharacterPacketVisitor>, public InworldPacketVisitor
	{
	public:
		FInworldCharacterPacketVisitor()
			: FInworldCharacterPacketVisitor(nullptr)
		{}
		FInworldCharacterPacketVisitor(class UInworldCharacter* InCharacter)
			: Character(InCharacter)
		{}
		virtual ~FInworldCharacterPacketVisitor() = default;

		virtual void Visit(const FInworldTextEvent& Event) override;
		virtual void Visit(const FInworldAudioDataEvent& Event) override;
		virtual void Visit(const FInworldSilenceEvent& Event) override;
		virtual void Visit(const FInworldControlEvent& Event) override;
		virtual void Visit(const FInworldEmotionEvent& Event) override;
		virtual void Visit(const FInworldCustomEvent& Event) override;

	private:
		UInworldCharacter* Character;
	};
	
	TSharedRef<FInworldCharacterPacketVisitor> PacketVisitor;
};

UINTERFACE(MinimalAPI, BlueprintType)
class UInworldCharacterOwnerInterface : public UInterface
{
	GENERATED_BODY()
};

class INWORLDAIINTEGRATION_API IInworldCharacterOwnerInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inworld")
	UInworldCharacter* GetInworldCharacter() const;
};

namespace Inworld
{
	TArray<FString> CharactersToAgentIds(const TArray<UInworldCharacter*>& InworldCharacters);
}
