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
#include "InworldPlayer.generated.h"

class UInworldSession;
class UInworldCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInworldPlayerTargetCharacterAdded, UInworldCharacter*, Character);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInworldPlayerTargetCharacterAddedNative, UInworldCharacter* /*Character*/);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInworldPlayerTargetCharacterRemoved, UInworldCharacter*, Character);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInworldPlayerTargetCharacterRemovedNative, UInworldCharacter* /*Character*/);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInworldPlayerTargetCharactersChanged);
DECLARE_MULTICAST_DELEGATE(FOnInworldPlayerTargetCharactersChangedNative);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInworldPlayerConversationChanged);
DECLARE_MULTICAST_DELEGATE(FOnInworldPlayerConversationChangedNative);

UCLASS(BlueprintType)
class INWORLDAIINTEGRATION_API UInworldPlayer : public UObject
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
	UFUNCTION(BlueprintCallable, Category = "Session")
	void SetSession(UInworldSession* InSession);
	UFUNCTION(BlueprintPure, Category = "Session")
	UInworldSession* GetSession() const { return Session; }

	UFUNCTION(BlueprintCallable, Category = "Message|Text")
	void SendTextMessageToConversation(const FString& Text);
	UFUNCTION(BlueprintCallable, Category = "Message|Trigger")
	void SendTriggerToConversation(const FString& Name, const TMap<FString, FString>& Params);
	UFUNCTION(BlueprintCallable, Category = "Message|Audio")
	void SendAudioSessionStartToConversation(EInworldMicrophoneMode MicrophoneMode = EInworldMicrophoneMode::OPEN_MIC);
	UFUNCTION(BlueprintCallable, Category = "Message|Audio")
	void SendAudioSessionStopToConversation();
	UFUNCTION(BlueprintCallable, Category = "Message|Audio")
	void SendSoundMessageToConversation(const TArray<uint8>& Input, const TArray<uint8>& Output);

public:
	UFUNCTION(BlueprintCallable, Category = "Player")
	TScriptInterface<IInworldPlayerOwnerInterface> GetInworldPlayerOwner();

	UFUNCTION(BlueprintCallable, Category = "Target")
	const TArray<UInworldCharacter*>& GetTargetCharacters() const { return TargetCharacters; }

	UFUNCTION(BlueprintCallable, Category = "Target")
	void AddTargetCharacter(UInworldCharacter* TargetCharacter);

	UFUNCTION(BlueprintCallable, Category = "Target")
	void RemoveTargetCharacter(UInworldCharacter* TargetCharacter);

	UFUNCTION(BlueprintCallable, Category = "Target")
	void ClearAllTargetCharacters();

	UPROPERTY(BlueprintAssignable, Category = "Target")
	FOnInworldPlayerTargetCharacterAdded OnTargetCharacterAddedDelegate;
	FOnInworldPlayerTargetCharacterAddedNative& OnTargetCharacterAdded() { return OnTargetCharacterAddedDelegateNative; }

	UPROPERTY(BlueprintAssignable, Category = "Target")
	FOnInworldPlayerTargetCharacterRemoved OnTargetCharacterRemovedDelegate;
	FOnInworldPlayerTargetCharacterRemovedNative& OnTargetCharacterRemoved() { return OnTargetCharacterRemovedDelegateNative; }

	UPROPERTY(BlueprintAssignable, Category = "Target")
	FOnInworldPlayerTargetCharactersChanged OnTargetCharactersChangedDelegate;
	FOnInworldPlayerTargetCharactersChangedNative& OnTargetCharactersChanged() { return OnTargetCharactersChangedDelegateNative; }

	UFUNCTION(BlueprintPure, Category = "Conversation")
	const FString& GetConversationId() const { return ConversationId; }

	UPROPERTY(BlueprintAssignable, Category = "Conversation")
	FOnInworldPlayerConversationChanged OnConversationChangedDelegate;
	FOnInworldPlayerConversationChangedNative& OnConversationChanged() { return OnConversationChangedDelegateNative; }

	bool HasAudioSession() const { return bHasAudioSession; }
	EInworldMicrophoneMode GetMicMode() const { return MicMode; }

private:
	void UpdateConversation();

private:
	UPROPERTY(Replicated)
	UInworldSession* Session;

	UPROPERTY(Replicated)
	TArray<UInworldCharacter*> TargetCharacters;

	FOnInworldPlayerTargetCharacterAddedNative OnTargetCharacterAddedDelegateNative;
	FOnInworldPlayerTargetCharacterRemovedNative OnTargetCharacterRemovedDelegateNative;
	FOnInworldPlayerTargetCharactersChangedNative OnTargetCharactersChangedDelegateNative;

	FString ConversationId;
	FOnInworldPlayerConversationChangedNative OnConversationChangedDelegateNative;

	bool bHasAudioSession = false;
	EInworldMicrophoneMode MicMode = EInworldMicrophoneMode::UNKNOWN;
};

UINTERFACE(MinimalAPI, BlueprintType)
class UInworldPlayerOwnerInterface : public UInterface
{
	GENERATED_BODY()
};

class INWORLDAIINTEGRATION_API IInworldPlayerOwnerInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inworld")
	UInworldPlayer* GetInworldPlayer() const;
};