/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "AudioCaptureCore.h"
#include "AudioDevice.h"
#include "Containers/ContainerAllocationPolicies.h"

#include "InworldPlayerAudioCaptureComponent.generated.h"

class UInworldApiSubsystem;
class UInworldPlayerComponent;
class UInworldCharacterComponent;
class USoundWave;
class UAudioCaptureComponent;

USTRUCT()
struct FPlayerVoiceCaptureInfoRep
{
    GENERATED_BODY()

    UPROPERTY()
	TArray<uint8> MicSoundData;
    UPROPERTY()
	TArray<uint8> OutputSoundData;
};

struct FInworldAudioCapture
{
public:
    FInworldAudioCapture(UObject* InOwner, TFunction<void(const TArray<uint8>& AudioData)> InCallback)
        : Owner(InOwner)
        , Callback(InCallback)
    {}
    virtual ~FInworldAudioCapture() {}

    virtual void RequestCapturePermission() {}
    virtual bool HasCapturePermission() const { return true; }

    virtual void StartCapture() = 0;
    virtual void StopCapture() = 0;

    virtual void SetCaptureDeviceById(const FString& DeviceId) = 0;

protected:
    UObject* Owner;
    TFunction<void(const TArray<uint8>& AudioData)> Callback;
};

UCLASS(ClassGroup = (Inworld), meta = (BlueprintSpawnableComponent))
class INWORLDAIINTEGRATION_API UInworldPlayerAudioCaptureComponent : public UActorComponent
{
	GENERATED_BODY()

public:
    UInworldPlayerAudioCaptureComponent(const FObjectInitializer& ObjectInitializer);

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    bool IsLocallyControlled() const;

private:
    void EvaluateVoiceCapture();

    UFUNCTION()
    void OnInworldConnectionStateChanged(EInworldConnectionState ConnectionState);

public:
    UFUNCTION(BlueprintCallable, Category = "Volume", meta=(DeprecatedFunction, DeprecationMessage="SetVolumeMultiplier is deprecated, use SetMuted instead."))
    void SetVolumeMultiplier(float InVolumeMultiplier) { bMuted = InVolumeMultiplier == 0.f; }

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetMuted(bool bInMuted) { ServerSetMuted(bInMuted); }

    UFUNCTION(Server, Reliable, Category = "Audio")
    void ServerSetMuted(bool bInMuted);

    UFUNCTION(BlueprintCallable, Category = "Devices")
    void SetCaptureDeviceById(const FString& DeviceId);

private:
    void StartCapture();
    void StopCapture();

    UFUNCTION(Server, Reliable)
    void Server_ProcessVoiceCaptureChunk(FPlayerVoiceCaptureInfoRep PlayerVoiceCaptureInfo);

protected:
    UPROPERTY(EditDefaultsOnly, Category = "Filter")
	bool bEnableAEC = false;

    UPROPERTY(EditDefaultsOnly, Category = "Pixel Stream")
    bool bPixelStream = false;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
    bool bMuted = false;

private:
	UFUNCTION()
	void Rep_ServerCapturingVoice();

	UPROPERTY(ReplicatedUsing=Rep_ServerCapturingVoice)
	bool bServerCapturingVoice = false;

    TAtomic<bool> bCapturingVoice = false;

	TWeakObjectPtr<UInworldApiSubsystem> InworldSubsystem;
    TWeakObjectPtr<UInworldPlayerComponent> PlayerComponent;

    TSharedPtr<FInworldAudioCapture> InputAudioCapture;
    TSharedPtr<FInworldAudioCapture> OutputAudioCapture;

    struct FAudioBuffer
    {
        FCriticalSection CriticalSection;
        TArray<uint8, TAlignedHeapAllocator<8>> Data;
    };

    FAudioBuffer InputBuffer;
    FAudioBuffer OutputBuffer;

    void OnPlayerTargetSet(UInworldCharacterComponent* Target);
    void OnPlayerTargetClear(UInworldCharacterComponent* Target);

    FDelegateHandle PlayerTargetSetHandle;
    FDelegateHandle PlayerTargetClearHandle;

    struct FPlayerAudioTarget
    {
        TArray<FString> AgentIds;

    } PlayerAudioTarget;

#if defined(WITH_GAMEPLAY_DEBUGGER) && WITH_GAMEPLAY_DEBUGGER
    friend class FInworldGameplayDebuggerCategory;
#endif
};
