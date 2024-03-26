/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

 // to avoid compile errors in windows unity build
#undef PlaySound

#include "InworldPlayerAudioCaptureComponent.h"
#include "InworldPlayerComponent.h"
#include "AudioMixerDevice.h"
#include "AudioMixerSubmix.h"
#include "AudioResampler.h"
#include "InworldApi.h"
#include "InworldAIPlatformModule.h"

#include "Runtime/Launch/Resources/Version.h"

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 1
#include "ISubmixBufferListener.h"
#endif

#if defined(INWORLD_PIXEL_STREAMING)
#include "IPixelStreamingModule.h"
#include "IPixelStreamingAudioConsumer.h"
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 1
#include "IPixelStreamingStreamer.h"
#endif
#endif

#include <Algo/Accumulate.h>
#include <Net/UnrealNetwork.h>
#include <GameFramework/PlayerController.h>

constexpr uint32 gSamplesPerSec = 16000;
constexpr uint32 gNumChannels = 1;

void ConvertAudioToInworldFormat(const float* AudioData, int32 NumFrames, int32 NumChannels, int32 SampleRate, TArray<uint16>& OutData)
{
    TArray<float> MutableAudioData{ AudioData, NumFrames };
    if (NumChannels != gNumChannels)
    {
        int32 DataOffset = 0;
        for (int32 CurrentFrame = 0; CurrentFrame < MutableAudioData.Num(); CurrentFrame++)
        {
            MutableAudioData[CurrentFrame] = Algo::Accumulate(TArray<float>{ AudioData + DataOffset , NumChannels }, 0.f) / NumChannels;

            DataOffset += NumChannels;
        }
    }

    if (SampleRate != gSamplesPerSec)
    {
        Audio::FAlignedFloatBuffer InputBuffer(MutableAudioData.GetData(), NumFrames);

        Audio::FResamplingParameters ResamplerParams = {
            Audio::EResamplingMethod::Linear,
            gNumChannels,
            (float)SampleRate,
            (float)gSamplesPerSec,
            InputBuffer
        };

        Audio::FAlignedFloatBuffer OutputBuffer;
        OutputBuffer.AddUninitialized(Audio::GetOutputBufferSize(ResamplerParams));

        Audio::FResamplerResults ResamplerResults;
        ResamplerResults.OutBuffer = &OutputBuffer;

        if (Audio::Resample(ResamplerParams, ResamplerResults))
        {
            MutableAudioData = { ResamplerResults.OutBuffer->GetData(), ResamplerResults.OutputFramesGenerated };
        }
    }

    OutData.SetNumUninitialized(MutableAudioData.Num());

    for (int32 CurrentFrame = 0; CurrentFrame < OutData.Num(); CurrentFrame++)
    {
        OutData[CurrentFrame] = MutableAudioData[CurrentFrame] * 32767; // 2^15, uint16
    }
}

struct FInworldMicrophoneAudioCapture : public FInworldAudioCapture
{
public:
    FInworldMicrophoneAudioCapture(UObject* InOwner, TFunction<void(const TArray<uint8>& AudioData)> InCallback)
        : FInworldAudioCapture(InOwner, InCallback) {}

    virtual void RequestCapturePermission();
    virtual bool HasCapturePermission() const override;
    virtual void StartCapture() override;
    virtual void StopCapture() override;

    virtual void SetCaptureDeviceById(const FString& DeviceId) override;

private:
    void OnAudioCapture(const float* AudioData, int32 NumFrames, int32 NumChannels, int32 SampleRate);

    Audio::FAudioCapture AudioCapture;
    Audio::FAudioCaptureDeviceParams AudioCaptureDeviceParams;

    mutable Inworld::Platform::Permission MicPermission = Inworld::Platform::Permission::UNDETERMINED;
};

struct FInworldPixelStreamAudioCapture : public FInworldAudioCapture
#if defined(INWORLD_PIXEL_STREAMING)
    , public IPixelStreamingAudioConsumer
#endif
{
public:
    FInworldPixelStreamAudioCapture(UObject* InOwner, TFunction<void(const TArray<uint8>& AudioData)> InCallback)
        : FInworldAudioCapture(InOwner, InCallback) {}

    virtual void StartCapture() override;
    virtual void StopCapture() override;

    // TODO: Allow switching streamer by Id. (Most likely not needed)
    virtual void SetCaptureDeviceById(const FString& DeviceId) override {}

#if defined(INWORLD_PIXEL_STREAMING)
    // IPixelStreamingAudioConsumer
    void ConsumeRawPCM(const int16_t* AudioData, int InSampleRate, size_t NChannels, size_t NFrames) override;
    void OnConsumerAdded() override {};
    void OnConsumerRemoved() override { AudioSink = nullptr; }
    // ~ IPixelStreamingAudioConsumer

    class IPixelStreamingAudioSink* AudioSink;
#endif
};

struct FInworldSubmixAudioCapture : public FInworldAudioCapture, public ISubmixBufferListener
{
public:
    FInworldSubmixAudioCapture(UObject* InOwner, TFunction<void(const TArray<uint8>& AudioData)> InCallback)
        : FInworldAudioCapture(InOwner, InCallback) {}

    virtual void StartCapture() override;
    virtual void StopCapture() override;

    // TODO: Allow switching submixes by Id. (Most likely not needed)
    virtual void SetCaptureDeviceById(const FString& DeviceId) override {}

    // ISubmixBufferListener
    void OnNewSubmixBuffer(const USoundSubmix* OwningSubmix, float* AudioData, int32 NumSamples, int32 NumChannels, const int32 InSampleRate, double AudioClock) override;
    // ~ ISubmixBufferListener
};

UInworldPlayerAudioCaptureComponent::UInworldPlayerAudioCaptureComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bTickEvenWhenPaused = true;
}

void UInworldPlayerAudioCaptureComponent::BeginPlay()
{
    Super::BeginPlay();

    SetIsReplicated(true);

    if (GetOwnerRole() == ROLE_Authority)
    {
        InworldSubsystem = GetWorld()->GetSubsystem<UInworldApiSubsystem>();

        InworldSubsystem->OnConnectionStateChanged.AddDynamic(this, &UInworldPlayerAudioCaptureComponent::OnInworldConnectionStateChanged);

        PlayerComponent = Cast<UInworldPlayerComponent>(GetOwner()->GetComponentByClass(UInworldPlayerComponent::StaticClass()));
        if (ensureMsgf(PlayerComponent.IsValid(), TEXT("UInworldPlayerAudioCaptureComponent::BeginPlay: add InworldPlayerComponent.")))
        {
            PlayerTargetSetHandle = PlayerComponent->OnTargetSet.AddUObject(this, &UInworldPlayerAudioCaptureComponent::OnPlayerTargetSet);
            PlayerTargetClearHandle = PlayerComponent->OnTargetClear.AddUObject(this, &UInworldPlayerAudioCaptureComponent::OnPlayerTargetClear);
        }

        PrimaryComponentTick.SetTickFunctionEnable(false);
    }
    
    if (IsLocallyControlled())
    {
        auto OnInputCapture = [this](const TArray<uint8>& AudioData)
            {
                if (bCapturingVoice)
                {
                    FScopeLock InputScopedLock(&InputBuffer.CriticalSection);
                    InputBuffer.Data.Append(AudioData);
                }
            };

        auto OnOutputCapture = [this](const TArray<uint8>& AudioData)
            {
                if (bCapturingVoice)
                {
                    FScopeLock OutputScopedLock(&OutputBuffer.CriticalSection);
                    OutputBuffer.Data.Append(AudioData);
                }
            };

        if (bPixelStream)
        {
            InputAudioCapture = MakeShared<FInworldPixelStreamAudioCapture>(this, OnInputCapture);
        }
        else
        {
            InputAudioCapture = MakeShared<FInworldMicrophoneAudioCapture>(this, OnInputCapture);
        }

        if (bEnableAEC)
        {
            OutputAudioCapture = MakeShared<FInworldSubmixAudioCapture>(this, OnOutputCapture);
        }

        InputAudioCapture->RequestCapturePermission();
        if (OutputAudioCapture.IsValid())
        {
            OutputAudioCapture->RequestCapturePermission();
        }

        PrimaryComponentTick.SetTickFunctionEnable(true);
    }
}

void UInworldPlayerAudioCaptureComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (GetOwnerRole() == ROLE_Authority)
    {
        InworldSubsystem->OnConnectionStateChanged.RemoveDynamic(this, &UInworldPlayerAudioCaptureComponent::OnInworldConnectionStateChanged);
    }

    if (PlayerComponent.IsValid())
    {
        PlayerComponent->OnTargetSet.Remove(PlayerTargetSetHandle);
        PlayerComponent->OnTargetClear.Remove(PlayerTargetClearHandle);
    }

    if (bCapturingVoice)
    {
        StopCapture();
    }

    Super::EndPlay(EndPlayReason);
}

void UInworldPlayerAudioCaptureComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    EvaluateVoiceCapture();

    {   
        FScopeLock InputScopedLock(&InputBuffer.CriticalSection);
        FScopeLock OutputScopedLock(&OutputBuffer.CriticalSection);

        constexpr int32 SampleSendSize = (gSamplesPerSec / 10) * 2; // 0.1s of data per send, mult by 2 from Buffer (uint8) to PCM (uint16)
        while (InputBuffer.Data.Num() > SampleSendSize && (!bEnableAEC || OutputBuffer.Data.Num() > SampleSendSize))
        {
            FPlayerVoiceCaptureInfoRep VoiceCaptureInfoRep;
            VoiceCaptureInfoRep.MicSoundData.Append(InputBuffer.Data.GetData(), SampleSendSize);
            FMemory::Memcpy(InputBuffer.Data.GetData(), InputBuffer.Data.GetData() + SampleSendSize, (InputBuffer.Data.Num() - SampleSendSize));
            InputBuffer.Data.SetNum(InputBuffer.Data.Num() - SampleSendSize);

            if (bEnableAEC)
            {
                VoiceCaptureInfoRep.OutputSoundData.Append(OutputBuffer.Data.GetData(), SampleSendSize);
                FMemory::Memcpy(OutputBuffer.Data.GetData(), OutputBuffer.Data.GetData() + SampleSendSize, (OutputBuffer.Data.Num() - SampleSendSize));
                OutputBuffer.Data.SetNum(OutputBuffer.Data.Num() - SampleSendSize);
            }

            Server_ProcessVoiceCaptureChunk(VoiceCaptureInfoRep);
        }
    }
}

void UInworldPlayerAudioCaptureComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UInworldPlayerAudioCaptureComponent, bServerCapturingVoice, COND_OwnerOnly);
}

void UInworldPlayerAudioCaptureComponent::EvaluateVoiceCapture()
{
    if (GetOwnerRole() == ROLE_Authority)
    {
        const bool bIsMicHot = !bMuted;
        const bool bIsWorldPlaying = !GetWorld()->IsPaused();
        const bool bHasTargetCharacter = PlayerAudioTarget.DesiredAgentIds.Num() != 0;
        const bool bHasActiveInworldSession = InworldSubsystem.IsValid() && (InworldSubsystem->GetConnectionState() == EInworldConnectionState::Connected || InworldSubsystem->GetConnectionState() == EInworldConnectionState::Reconnecting);

        const bool bShouldCaptureVoice = bIsMicHot && bIsWorldPlaying && bHasTargetCharacter && bHasActiveInworldSession;

        if (bShouldCaptureVoice && bServerCapturingVoice && (PlayerAudioTarget.DesiredAgentIds != PlayerAudioTarget.ActiveAgentIds))
        {
            InworldSubsystem->StopAudioSessionMultiAgent(PlayerAudioTarget.ActiveAgentIds);
            InworldSubsystem->StartAudioSessionMultiAgent(PlayerAudioTarget.DesiredAgentIds, GetOwner());
            PlayerAudioTarget.ActiveAgentIds = PlayerAudioTarget.DesiredAgentIds;
        }
        else if (bShouldCaptureVoice != bServerCapturingVoice)
        {
            if (bShouldCaptureVoice)
            {
                InworldSubsystem->StartAudioSessionMultiAgent(PlayerAudioTarget.DesiredAgentIds, GetOwner());
                PlayerAudioTarget.ActiveAgentIds = PlayerAudioTarget.DesiredAgentIds;
            }
            else
            {
                InworldSubsystem->StopAudioSessionMultiAgent(PlayerAudioTarget.ActiveAgentIds);
                PlayerAudioTarget.ActiveAgentIds.Empty();
            }

            bServerCapturingVoice = bShouldCaptureVoice;

            if (IsLocallyControlled())
            {
                Rep_ServerCapturingVoice();
            }
        }
    }
}

void UInworldPlayerAudioCaptureComponent::OnInworldConnectionStateChanged(EInworldConnectionState ConnectionState)
{
    EvaluateVoiceCapture();
}

void UInworldPlayerAudioCaptureComponent::ServerSetMuted_Implementation(bool bInMuted)
{
    bMuted = bInMuted;
    EvaluateVoiceCapture();
}

void UInworldPlayerAudioCaptureComponent::SetCaptureDeviceById(const FString& DeviceId)
{
    if (!IsInAudioThread())
    {
        FAudioThread::RunCommandOnAudioThread([this, DeviceId]()
        {
            SetCaptureDeviceById(DeviceId);
        });
        return;
    }

    if (InputAudioCapture.IsValid())
    {
        InputAudioCapture->SetCaptureDeviceById(DeviceId);
    }

    const bool bWasCapturingVoice = bCapturingVoice;

    if (bWasCapturingVoice)
    {
        StopCapture();
        StartCapture();
    }
}

void UInworldPlayerAudioCaptureComponent::StartCapture()
{
    if (!IsInAudioThread())
    {
        FAudioThread::RunCommandOnAudioThread([this]()
            {
                StartCapture();
            });
        return;
    }

    if (bCapturingVoice)
    {
        return;
    }

    if (!InputAudioCapture.IsValid() || !InputAudioCapture->HasCapturePermission())
    {
        return;
    }

    if (OutputAudioCapture.IsValid() && !InputAudioCapture->HasCapturePermission())
    {
        return;
    }

    InputAudioCapture->StartCapture();
    if (OutputAudioCapture.IsValid())
    {
        OutputAudioCapture->StartCapture();
    }

    bCapturingVoice = true;
}

void UInworldPlayerAudioCaptureComponent::StopCapture()
{
    if (!IsInAudioThread())
    {
        FAudioThread::RunCommandOnAudioThread([this]()
            {
                StopCapture();
            });
        return;
    }

    if (!bCapturingVoice)
    {
        return;
    }

    InputAudioCapture->StopCapture();
    if (OutputAudioCapture)
    {
        OutputAudioCapture->StopCapture();
    }

    bCapturingVoice = false;

    FScopeLock InputScopedLock(&InputBuffer.CriticalSection);
    FScopeLock OutputScopedLock(&OutputBuffer.CriticalSection);
    InputBuffer.Data.Empty();
    OutputBuffer.Data.Empty();
}

void UInworldPlayerAudioCaptureComponent::Server_ProcessVoiceCaptureChunk_Implementation(FPlayerVoiceCaptureInfoRep PlayerVoiceCaptureInfo)
{
    if (PlayerAudioTarget.ActiveAgentIds.Num() != 0)
    {
        if (bEnableAEC)
        {
            InworldSubsystem->SendAudioDataMessageWithAEC(PlayerAudioTarget.ActiveAgentIds, PlayerVoiceCaptureInfo.MicSoundData, PlayerVoiceCaptureInfo.OutputSoundData);
        }
        else
        {
            InworldSubsystem->SendAudioDataMessage(PlayerAudioTarget.ActiveAgentIds, PlayerVoiceCaptureInfo.MicSoundData);
        }
    }
}

bool UInworldPlayerAudioCaptureComponent::IsLocallyControlled() const
{
    auto* Controller = Cast<APlayerController>(GetOwner()->GetInstigatorController());
    return Controller && Controller->IsLocalController();
}

void UInworldPlayerAudioCaptureComponent::OnPlayerTargetSet(UInworldCharacterComponent* Target)
{
    if (PlayerAudioTarget.DesiredAgentIds.Contains(Target->GetAgentId()))
    {
        return;
    }
    PlayerAudioTarget.DesiredAgentIds.Add(Target->GetAgentId());
    EvaluateVoiceCapture();
}

void UInworldPlayerAudioCaptureComponent::OnPlayerTargetClear(UInworldCharacterComponent* Target)
{
    if (!PlayerAudioTarget.DesiredAgentIds.Contains(Target->GetAgentId()))
    {
        return;
    }
    PlayerAudioTarget.DesiredAgentIds.RemoveSingle(Target->GetAgentId());
    EvaluateVoiceCapture();
}

void UInworldPlayerAudioCaptureComponent::Rep_ServerCapturingVoice()
{
    if (bServerCapturingVoice)
	{
        StartCapture();
    }
    else
    {
        StopCapture();
    }
}

void FInworldMicrophoneAudioCapture::RequestCapturePermission()
{
    FInworldAIPlatformModule& PlatformModule = FModuleManager::Get().LoadModuleChecked<FInworldAIPlatformModule>("InworldAIPlatform");
    Inworld::Platform::IMicrophone* Microphone = PlatformModule.GetMicrophone();
    if (Microphone->GetPermission() == Inworld::Platform::Permission::UNDETERMINED)
    {
        Microphone->RequestAccess([](bool Granted)
            {
                ensureMsgf(Granted, TEXT("FInworldMicrophoneAudioCapture::RequestCapturePermission: Platform has denied access to microphone."));
            });
    }
    else
    {
        const bool Granted = Microphone->GetPermission() == Inworld::Platform::Permission::GRANTED;
        ensureMsgf(Granted, TEXT("FInworldMicrophoneAudioCapture::RequestCapturePermission: Platform has denied access to microphone."));
    }
}

bool FInworldMicrophoneAudioCapture::HasCapturePermission() const
{
    if (MicPermission == Inworld::Platform::Permission::UNDETERMINED)
    {
        FInworldAIPlatformModule& PlatformModule = FModuleManager::Get().LoadModuleChecked<FInworldAIPlatformModule>("InworldAIPlatform");
        Inworld::Platform::IMicrophone* Microphone = PlatformModule.GetMicrophone();
        MicPermission = Microphone->GetPermission();
    }
    return MicPermission == Inworld::Platform::Permission::GRANTED;
}

void FInworldMicrophoneAudioCapture::StartCapture()
{
    if (!AudioCapture.IsStreamOpen())
    {
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 3
        Audio::FOnAudioCaptureFunction OnCapture = [this](const void* AudioData, int32 NumFrames, int32 NumChannels, int32 SampleRate, double StreamTime, bool bOverFlow)
            {
                OnAudioCapture((const float*)AudioData, NumFrames, NumChannels, SampleRate);
            };

        AudioCapture.OpenAudioCaptureStream(AudioCaptureDeviceParams, MoveTemp(OnCapture), 1024);
#else
        Audio::FOnCaptureFunction OnCapture = [this](const float* AudioData, int32 NumFrames, int32 NumChannels, int32 SampleRate, double StreamTime, bool bOverFlow)
            {
                OnAudioCapture(AudioData, NumFrames, NumChannels, SampleRate);
            };

        AudioCapture.OpenCaptureStream(AudioCaptureDeviceParams, MoveTemp(OnCapture), 1024);
#endif
    }

    if (AudioCapture.IsStreamOpen())
    {
        AudioCapture.StartStream();
    }
}

void FInworldMicrophoneAudioCapture::StopCapture()
{
    if (AudioCapture.IsStreamOpen())
    {
        AudioCapture.StopStream();
        AudioCapture.CloseStream();
    }
}

void FInworldMicrophoneAudioCapture::SetCaptureDeviceById(const FString& DeviceId)
{
    Audio::FAudioCaptureDeviceParams Params;

    if (DeviceId.IsEmpty())
    {
        Params = Audio::FAudioCaptureDeviceParams();
    }
    else
    {
        TArray<Audio::FCaptureDeviceInfo> CaptureDeviceInfos;
        AudioCapture.GetCaptureDevicesAvailable(CaptureDeviceInfos);

        int32 DeviceIndex = 0;
        int32 CaptureDeviceIndex = 0;
        Audio::FCaptureDeviceInfo OutInfo;
        bool bFoundMatchingInputDevice = false;
        while (CaptureDeviceIndex < CaptureDeviceInfos.Num())
        {
            AudioCapture.GetCaptureDeviceInfo(OutInfo, DeviceIndex);
            if (OutInfo.DeviceId == DeviceId)
            {
                Params.DeviceIndex = CaptureDeviceIndex;
                Params.bUseHardwareAEC = OutInfo.bSupportsHardwareAEC;
                bFoundMatchingInputDevice = true;
                break;
            }
            if (CaptureDeviceInfos.ContainsByPredicate([OutInfo](const Audio::FCaptureDeviceInfo& CaptureDeviceInfo) -> bool {
                return CaptureDeviceInfo.DeviceId == OutInfo.DeviceId;
            }))
            {
                CaptureDeviceIndex++;
            }
            DeviceIndex++;
        }
        if (!bFoundMatchingInputDevice)
        {
            return;
        }
    }

    if (AudioCaptureDeviceParams.DeviceIndex == Params.DeviceIndex)
    {
        return;
    }

    AudioCaptureDeviceParams = Params;
}

void FInworldMicrophoneAudioCapture::OnAudioCapture(const float* AudioData, int32 NumFrames, int32 NumChannels, int32 SampleRate)
{
    TArray<uint16> Buffer;
    ConvertAudioToInworldFormat(AudioData, NumFrames, NumChannels, SampleRate, Buffer);
    Callback({ (uint8*)Buffer.GetData(), Buffer.Num() * 2 });
}

void FInworldPixelStreamAudioCapture::StartCapture()
{
#if defined(INWORLD_PIXEL_STREAMING)
    IPixelStreamingModule& PixelStreamingModule = IPixelStreamingModule::Get();
    if (!PixelStreamingModule.IsReady())
    {
        return;
    }

    IPixelStreamingAudioSink* CandidateSink = nullptr;

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 1
    TSharedPtr<IPixelStreamingStreamer> Streamer = PixelStreamingModule.FindStreamer(PixelStreamingModule.GetDefaultStreamerID());
    if (!Streamer)
    {
        return;
    }
    CandidateSink = Streamer->GetUnlistenedAudioSink();
#else
    CandidateSink = PixelStreamingModule.GetUnlistenedAudioSink();
#endif

    if (CandidateSink == nullptr)
    {
        return;
    }

    AudioSink = CandidateSink;
    AudioSink->AddAudioConsumer(this);
#endif
}

void FInworldPixelStreamAudioCapture::StopCapture()
{
#if defined(INWORLD_PIXEL_STREAMING)
    if (AudioSink)
    {
        AudioSink->RemoveAudioConsumer(this);
    }

    AudioSink = nullptr;
#endif
}

#if defined(INWORLD_PIXEL_STREAMING)
void FInworldPixelStreamAudioCapture::ConsumeRawPCM(const int16_t* AudioData, int InSampleRate, size_t NChannels, size_t NFrames)
{
    TArray<float> fAudioData;
    fAudioData.SetNumUninitialized(NFrames * NChannels);
    for (int32 i = 0; i < fAudioData.Num(); i++)
    {
        fAudioData[i] = ((float)AudioData[i]) / 32767.f; // 2^15, uint16
    }
    TArray<uint16> Buffer;
    ConvertAudioToInworldFormat(fAudioData.GetData(), NFrames, NChannels, InSampleRate, Buffer);
    Callback({ (uint8*)Buffer.GetData(), Buffer.Num() * 2 });
}
#endif

void FInworldSubmixAudioCapture::StartCapture()
{
    Audio::FMixerDevice* MixerDevice = static_cast<Audio::FMixerDevice*>(Owner->GetWorld()->GetAudioDeviceRaw());
    if (MixerDevice)
    {
        Audio::FMixerSubmixPtr MasterSubmix = MixerDevice->GetMasterSubmix().Pin();
        if (MasterSubmix.IsValid())
        {
            MasterSubmix->RegisterBufferListener(this);
        }
    }
}

void FInworldSubmixAudioCapture::StopCapture()
{
    Audio::FMixerDevice* MixerDevice = static_cast<Audio::FMixerDevice*>(Owner->GetWorld()->GetAudioDeviceRaw());
    if (MixerDevice)
    {
        Audio::FMixerSubmixPtr MasterSubmix = MixerDevice->GetMasterSubmix().Pin();
        if (MasterSubmix.IsValid())
        {
            MasterSubmix->UnregisterBufferListener(this);
        }
    }
}

void FInworldSubmixAudioCapture::OnNewSubmixBuffer(const USoundSubmix* OwningSubmix, float* AudioData, int32 NumSamples, int32 NumChannels, const int32 SampleRate, double AudioClock)
{
    const int32 NumFrames = NumSamples / NumChannels;
    TArray<uint16> Buffer;
    ConvertAudioToInworldFormat(AudioData, NumFrames, NumChannels, SampleRate, Buffer);
    Callback({ (uint8*)Buffer.GetData(), Buffer.Num() * 2 });
}
