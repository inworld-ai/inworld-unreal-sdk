// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InworldSession.h"
#include "InworldSessionComponent.generated.h"


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
private:
	UFUNCTION()
	void OnRep_InworldSession();

	UPROPERTY(ReplicatedUsing = OnRep_InworldSession)
	UInworldSession* InworldSession;
};
