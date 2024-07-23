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
    UFUNCTION(BlueprintCallable, Category = "Volume", meta=(DeprecatedFunction, DeprecationMessage="SetVolumeMultiplier is deprecated, use SetMuted instead."))
    void SetVolumeMultiplier(float InVolumeMultiplier) { bMuted = InVolumeMultiplier == 0.f; }

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetMuted(bool bInMuted) { ServerSetMuted(bInMuted); }

    UFUNCTION(Server, Reliable, Category = "Audio")
    void ServerSetMuted(bool bInMuted);

    UFUNCTION(BlueprintCallable, Category = "Audio", meta=(DeprecatedFunction, DeprecationMessage="SetMicMode is deprecated, use SetAudioSessionMode instead."))
    void SetMicMode(EInworldMicrophoneMode InMicMode) { ServerSetMicMode(InMicMode); }

    UFUNCTION(Server, Reliable, Category = "Audio")
	void ServerSetMicMode(EInworldMicrophoneMode InMicMode);

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void SetAudioSessionMode(FInworldAudioSessionOptions InMode) { ServerSetAudioSessionMode(InMode); }

	UFUNCTION(Server, Reliable, Category = "Audio")
	void ServerSetAudioSessionMode(FInworldAudioSessionOptions InMode);

    UFUNCTION(BlueprintCallable, Category = "Devices")
    void SetCaptureDeviceById(const FString& DeviceId);

private:
    void StartCapture();
    void StopCapture();

    UFUNCTION(Server, Reliable)
    void Server_ProcessVoiceCaptureChunk(FPlayerVoiceCaptureInfoRep PlayerVoiceCaptureInfo);

protected:
    UPROPERTY(EditDefaultsOnly, Category = "Filter")
	bool bEnableAEC = true;

    UPROPERTY(EditDefaultsOnly, Category = "Pixel Stream")
    bool bPixelStream = false;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
    bool bMuted = false;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
	FInworldAudioSessionOptions AudioSessionMode;
    bool bIsAudioSessionModeDirty = false;

    bool bSessionPendingPause = false;

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
