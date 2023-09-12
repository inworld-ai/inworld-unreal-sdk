/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "AudioCaptureCore.h"
#include "AudioDevice.h"
#include "Containers/ContainerAllocationPolicies.h"
#include "InworldGameplayDebuggerCategory.h"

#include "Runtime/Launch/Resources/Version.h"
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 1
#include "ISubmixBufferListener.h"
#endif

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

UCLASS(ClassGroup = (Inworld), meta = (BlueprintSpawnableComponent))
class INWORLDAIINTEGRATION_API UInworldPlayerAudioCaptureComponent : public UActorComponent, public ISubmixBufferListener
{
	GENERATED_BODY()

public:
    UInworldPlayerAudioCaptureComponent();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    bool IsLocallyControlled() const;

public:
    UFUNCTION(BlueprintCallable, Category = "Volume")
    void SetVolumeMultiplier(float InVolumeMultiplier) { VolumeMultiplier = InVolumeMultiplier; }

    UFUNCTION(BlueprintCallable, Category = "Devices")
    void SetCaptureDeviceById(const FString& DeviceId);

private:
    void OpenStream();
    void CloseStream();

    void StartStream();
    void StopStream();

    void OnAudioCapture(const float* AudioData, int32 NumFrames, int32 NumChannels, int32 SampleRate, double StreamTime, bool bOverFlow);

    // ISubmixBufferListener
    void OnNewSubmixBuffer(const USoundSubmix* OwningSubmix, float* AudioData, int32 NumSamples, int32 NumChannels, const int32 InSampleRate, double AudioClock) override;
    // ~ ISubmixBufferListener

    UFUNCTION(Server, Reliable)
    void Server_ProcessVoiceCaptureChunk(FPlayerVoiceCaptureInfoRep PlayerVoiceCaptureInfo);

protected:
    UPROPERTY(EditAnywhere, Category = "Filter")
	bool bEnableAEC = false;

private:
	UFUNCTION()
	void Rep_ServerCapturingVoice();

	UPROPERTY(ReplicatedUsing=Rep_ServerCapturingVoice)
	bool bServerCapturingVoice = false;

    TAtomic<bool> bCapturingVoice = false;

	TWeakObjectPtr<UInworldApiSubsystem> InworldSubsystem;
    TWeakObjectPtr<UInworldPlayerComponent> PlayerComponent;

    Audio::FAudioCapture AudioCapture;
    Audio::FAudioCaptureDeviceParams AudioCaptureDeviceParams;

    FCriticalSection InputCriticalSection;
    TArray<int16, TAlignedHeapAllocator<16>> InputBuffer;

    FCriticalSection OutputCriticalSection;
    TArray<int16, TAlignedHeapAllocator<16>> OutputBuffer;

    float VolumeMultiplier = 1.f;

    void OnPlayerTargetSet(UInworldCharacterComponent* Target);
    void OnPlayerTargetClear(UInworldCharacterComponent* Target);

    FDelegateHandle PlayerTargetSetHandle;
    FDelegateHandle PlayerTargetClearHandle;

    friend class FInworldGameplayDebuggerCategory;
};
