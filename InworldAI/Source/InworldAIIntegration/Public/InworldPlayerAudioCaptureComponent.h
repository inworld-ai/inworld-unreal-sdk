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
#include "InworldEnums.h"
#include "InworldTypes.h"
#include "Containers/ContainerAllocationPolicies.h"

#include "InworldPlayerAudioCaptureComponent.generated.h"

class UInworldPlayer;
class USoundWave;
class UAudioCaptureComponent;

USTRUCT()
struct FPlayerVoiceCaptureInfoRep
{
    GENERATED_BODY()
    /** Microphone sound data. */
    UPROPERTY()
    TArray<uint8> MicSoundData;
    /** Output sound data. */
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

    virtual bool Initialize() = 0;

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

public:
    /**
     * Set the volume multiplier.
     * @param InVolumeMultiplier The volume multiplier value.
     */
    UFUNCTION(BlueprintCallable, Category = "Volume", meta=(DeprecatedFunction, DeprecationMessage="SetVolumeMultiplier is deprecated, use SetMuted instead."))
    void SetVolumeMultiplier(float InVolumeMultiplier) { bMuted = InVolumeMultiplier == 0.f; }

    /**
     * Set whether the audio is muted.
     * @param bInMuted True if audio is muted, false otherwise.
     */
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetMuted(bool bInMuted) { ServerSetMuted(bInMuted); }

    /**
     * Server-side function to set the muted state.
     * @param bInMuted True if audio is muted, false otherwise.
     */
    UFUNCTION(Server, Reliable, Category = "Audio")
    void ServerSetMuted(bool bInMuted);

    /**
     * Set the microphone mode.
     * @param InMicMode The microphone mode.
     */
    UFUNCTION(BlueprintCallable, Category = "Audio", meta=(DeprecatedFunction, DeprecationMessage="SetMicMode is deprecated, use SetAudioSessionOptions instead."))
    void SetMicMode(EInworldMicrophoneMode InMicMode) { ServerSetMicMode(InMicMode); }

    /**
     * Server-side function to set the microphone mode.
     * @param InMicMode The microphone mode.
     */
    UFUNCTION(Server, Reliable, Category = "Audio")
    void ServerSetMicMode(EInworldMicrophoneMode InMicMode);

    /**
     * Set the audio session options.
     * @param InMode The audio session options to set.
     */
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetAudioSessionOptions(FInworldAudioSessionOptions InMode) { ServerSetAudioSessionOptions(InMode); }

    /**
     * Server-side function to set the audio session options.
     * @param InMode The audio session options to set.
     */
    UFUNCTION(Server, Reliable, Category = "Audio")
    void ServerSetAudioSessionOptions(FInworldAudioSessionOptions InMode);

    /**
     * Set the capture device by its ID.
     * @param DeviceId The ID of the capture device to set.
     */
    UFUNCTION(BlueprintCallable, Category = "Devices")
    void SetCaptureDeviceById(const FString& DeviceId);

private:
    void StartCapture();
    void StopCapture();

    UFUNCTION(Server, Reliable)
    void Server_ProcessVoiceCaptureChunk(FPlayerVoiceCaptureInfoRep PlayerVoiceCaptureInfo);

protected:
    /**
     * Enable Acoustic Echo Cancellation (AEC) filter.
     */
    UPROPERTY(EditDefaultsOnly, Category = "Filter")
    bool bEnableAEC = true;

    /**
     * Enable Pixel Streaming.
     */
    UPROPERTY(EditDefaultsOnly, Category = "Pixel Stream")
    bool bPixelStream = false;

    /**
     * Whether audio is muted.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
    bool bMuted = false;

    /**
     * Audio session options.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
    FInworldAudioSessionOptions AudioSessionOptions;
    bool bIsAudioSessionOptionsDirty = false;

    /**
     * Player speech mode.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
    EInworldPlayerSpeechMode PlayerSpeechMode;

    /**
     * Player speech options (hidden if PlayerSpeechMode is Default).
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio", meta = (EditCondition = "PlayerSpeechMode != EInworldPlayerSpeechMode::Default", EditConditionHides))
    FInworldPlayerSpeechOptions PlayerSpeechOptions;

private:
    UFUNCTION()
    void Rep_ServerCapturingVoice();

    UPROPERTY(ReplicatedUsing=Rep_ServerCapturingVoice)
    bool bServerCapturingVoice = false;

    TAtomic<bool> bCapturingVoice = false;

    TWeakObjectPtr<UInworldPlayer> InworldPlayer;
    FDelegateHandle OnPlayerConversationChanged;

    FDelegateHandle OnSessionPrePause;
    FDelegateHandle OnSessionConnectionStateChanged;
    FDelegateHandle OnSessionLoaded;
    bool bSessionPendingPause = false;

    TSharedPtr<FInworldAudioCapture> InputAudioCapture;
    TSharedPtr<FInworldAudioCapture> OutputAudioCapture;

    struct FAudioBuffer
    {
        FCriticalSection CriticalSection;
        TArray<uint8, TAlignedHeapAllocator<8>> Data;
    };

    FAudioBuffer InputBuffer;
    FAudioBuffer OutputBuffer;

#if defined(WITH_GAMEPLAY_DEBUGGER) && WITH_GAMEPLAY_DEBUGGER
    friend class FInworldGameplayDebuggerCategory;
#endif
};
