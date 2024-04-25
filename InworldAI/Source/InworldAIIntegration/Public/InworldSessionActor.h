// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InworldSessionActor.generated.h"

class UInworldSession;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInworldSessionCreated);
DECLARE_MULTICAST_DELEGATE(FOnInworldSessionCreatedNative);

UCLASS()
class INWORLDAIINTEGRATION_API AInworldSessionActor : public AActor, public IInworldSessionOwnerInterface
{
	GENERATED_BODY()
	
public:
	AInworldSessionActor();

	// IInworldSessionOwnerInterface
	virtual UInworldSession* GetInworldSession_Implementation() const override { return InworldSession; }
	// ~IInworldSessionOwnerInterface

	UPROPERTY(BlueprintAssignable, Category = "Session")
	FOnInworldSessionCreated OnSessionCreatedDelegate;
	FOnInworldSessionCreatedNative& OnSessionCreated() { return OnSessionCreatedDelegateNative; }

private:
	FOnInworldSessionCreatedNative OnSessionCreatedDelegateNative;

protected:
	virtual void PreInitializeComponents() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

private:
	UFUNCTION()
	void OnRep_InworldSession();

	UPROPERTY(ReplicatedUsing=OnRep_InworldSession)
	UInworldSession* InworldSession;
};
