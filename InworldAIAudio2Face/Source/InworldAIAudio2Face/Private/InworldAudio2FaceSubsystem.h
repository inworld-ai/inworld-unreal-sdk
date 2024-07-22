// Copyright 2023 Theai, Inc. (DBA Inworld) All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "InworldApi.h"
#include "InworldCharacterPlaybackA2F.h"
#include "InworldCharacterPlaybackA2F_2.h"
#include "InworldAudio2FaceSubsystem.generated.h"

UCLASS(BlueprintType, Config = InworldAI)
class INWORLDAIAUDIO2FACE_API UInworldAudio2FaceSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:

	UInworldAudio2FaceSubsystem();

	UFUNCTION(BlueprintCallable, Category = "Inworld")
	bool IsA2FClient() const
	{
		return FParse::Param(FCommandLine::Get(), TEXT("clientA2f"));
	}

	UFUNCTION(BlueprintCallable, Category = "Inworld")
	bool IsA2FBackend() const
	{
		return FParse::Param(FCommandLine::Get(), TEXT("backendA2F"));
	}

	virtual void Initialize(FSubsystemCollectionBase& Collection) override
	{
		Super::Initialize(Collection);
		if (IsA2FClient())
		{
			GetWorld()->GetSubsystem<UInworldApiSubsystem>()->A2FPlayback = UInworldCharacterPlaybackA2F::StaticClass();
		}
		else if (IsA2FBackend())
		{
			GetWorld()->GetSubsystem<UInworldApiSubsystem>()->A2FPlayback = UInworldCharacterPlaybackA2F_2::StaticClass();
		}
	}

	virtual bool DoesSupportWorldType(EWorldType::Type WorldType) const override
	{
		return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
	}

	UFUNCTION(BlueprintCallable, Category = "Audio2Face")
	void Start(const FString& URL);

	UFUNCTION(BlueprintCallable, Category = "Audio2Face")
	void Stop();

	UFUNCTION(BlueprintCallable, Category = "Audio2Face")
	void Update();

	UFUNCTION(BlueprintCallable, Category = "Audio2Face")
	void SendAudio(const TArray<uint8>& Audio);

	UFUNCTION(BlueprintCallable, Category = "Audio2Face")
	void EndAudio();

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInworldAudio2FaceAnimDataHeader, const FAudio2FaceAnimDataHeader&, Header);
	UPROPERTY(BlueprintAssignable, Category = "Audio2Face")
	FOnInworldAudio2FaceAnimDataHeader OnInworldAudio2FaceAnimDataHeader;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInworldAudio2FaceAnimDataContent, const FAudio2FaceAnimDataContent&, Content);
	UPROPERTY(BlueprintAssignable, Category = "Audio2Face")
	FOnInworldAudio2FaceAnimDataContent OnInworldAudio2FaceAnimDataContent;

private:
	FInworldAudio2Face Audio2Face;
};
