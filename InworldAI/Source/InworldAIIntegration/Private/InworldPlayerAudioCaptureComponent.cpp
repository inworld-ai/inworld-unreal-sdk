/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
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

#include <Net/UnrealNetwork.h>
#include <GameFramework/PlayerController.h>

constexpr uint32 gSamplesPerSec = 16000;

struct FInworldMicrophoneAudioCapture : public FInworldAudioCapture
{
public:
    FInworldMicrophoneAudioCapture(UObject* InOwner, TFunction<void(const TArray<uint8>& AudioData)> InCallback)
        : FInworldAudioCapture(InOwner, InCallback) {}

    virtual void StartCapture() override;
    virtual void StopCapture() override;

    virtual void SetCaptureDeviceById(const FString& DeviceId) override;

private:
    void OnAudioCapture(const float* AudioData, int32 NumFrames, int32 NumChannels, int32 SampleRate);

    Audio::FAudioCapture AudioCapture;
    Audio::FAudioCaptureDeviceParams AudioCaptureDeviceParams;
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

    if (GetOwnerRole() == ROLE_Authority)
    {
        InworldSubsystem = GetWorld()->GetSubsystem<UInworldApiSubsystem>();

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
        FInworldAIPlatformModule& PlatformModule = FModuleManager::Get().LoadModuleChecked<FInworldAIPlatformModule>("InworldAIPlatform");
        Inworld::Platform::IMicrophone* Microphone = PlatformModule.GetMicrophone();
        if (Microphone->GetPermission() == Inworld::Platform::Permission::UNDETERMINED)
        {
            Microphone->RequestAccess([](bool Granted)
            {
                ensureMsgf(Granted, TEXT("UInworldPlayerAudioCaptureComponent::BeginPlay: Platform has denied access to microphone."));
            });
        }
        else
        {
            const bool Granted = Microphone->GetPermission() == Inworld::Platform::Permission::GRANTED;
            ensureMsgf(Granted, TEXT("UInworldPlayerAudioCaptureComponent::BeginPlay: Platform has denied access to microphone."));
        }

        PrimaryComponentTick.SetTickFunctionEnable(true);
    }
}

void UInworldPlayerAudioCaptureComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
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

    if (InworldSubsystem.IsValid())
    {
        if (TickType == ELevelTick::LEVELTICK_PauseTick || InworldSubsystem->GetConnectionState() != EInworldConnectionState::Connected)
        {
            if(bServerCapturingVoice)
            {
                bServerCapturingVoice = false;
                PlayerComponent->StopAudioSessionWithTarget();
            }

            if (IsLocallyControlled())
            {
                Rep_ServerCapturingVoice();
            }
            return;
        }

        // restore voice capture after pause
        if (PlayerComponent->GetTargetCharacter() && !bCapturingVoice)
        {
            if (!bServerCapturingVoice)
            {
                PlayerComponent->StartAudioSessionWithTarget();
                bServerCapturingVoice = true;
            }

            if (IsLocallyControlled())
            {
                Rep_ServerCapturingVoice();
            }
        }
    }

    {   
        FScopeLock InputScopedLock(&InputBuffer.CriticalSection);
        FScopeLock OutputScopedLock(&OutputBuffer.CriticalSection);

        constexpr int32 SampleSendSize = (gSamplesPerSec / 10) * 2; // 0.1s of data per send, mult by 2 from Buffer (uint8) to PCM (uint16)
        while (InputBuffer.Data.Num() > SampleSendSize && (!bEnableAEC || OutputBuffer.Data.Num() > SampleSendSize))
        {
            FPlayerVoiceCaptureInfoRep VoiceCaptureInfoRep;
            if (bMuted)
            {
                VoiceCaptureInfoRep.MicSoundData.AddZeroed(SampleSendSize);
            }
            else
            {
                VoiceCaptureInfoRep.MicSoundData.Append(InputBuffer.Data.GetData(), SampleSendSize);
            }
            FMemory::Memcpy(InputBuffer.Data.GetData(), InputBuffer.Data.GetData() + SampleSendSize, (InputBuffer.Data.Num() - SampleSendSize));
            InputBuffer.Data.SetNum(InputBuffer.Data.Num() - SampleSendSize);

            if (bEnableAEC)
            {
                if (bMuted)
                {
                    VoiceCaptureInfoRep.MicSoundData.AddZeroed(SampleSendSize);
                }
                else
                {
                    VoiceCaptureInfoRep.OutputSoundData.Append(OutputBuffer.Data.GetData(), SampleSendSize);
                }
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

    InputAudioCapture->SetCaptureDeviceById(DeviceId);

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
	if (bEnableAEC)
	{
        PlayerComponent->SendAudioDataMessageWithAECToTarget(PlayerVoiceCaptureInfo.MicSoundData, PlayerVoiceCaptureInfo.OutputSoundData);
	}
	else
	{
        PlayerComponent->SendAudioDataMessageToTarget(PlayerVoiceCaptureInfo.MicSoundData);
	}
}

bool UInworldPlayerAudioCaptureComponent::IsLocallyControlled() const
{
    auto* Controller = Cast<APlayerController>(GetOwner()->GetInstigatorController());
    return Controller && Controller->IsLocalController();
}

void UInworldPlayerAudioCaptureComponent::OnPlayerTargetSet(UInworldCharacterComponent* Target)
{
    if (Target)
	{
        InworldSubsystem->StartAudioSession(Target->GetAgentId());
        bServerCapturingVoice = true;
    }

    if (IsLocallyControlled())
    {
        Rep_ServerCapturingVoice();
    }
}

void UInworldPlayerAudioCaptureComponent::OnPlayerTargetClear(UInworldCharacterComponent* Target)
{
    if (Target)
    {
        InworldSubsystem->StopAudioSession(Target->GetAgentId());
        bServerCapturingVoice = false;
    }

    if (IsLocallyControlled())
    {
        Rep_ServerCapturingVoice();
    }
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
    const int32 DownsampleRate = SampleRate / gSamplesPerSec;
    const int32 nFrames = NumFrames / DownsampleRate;

    TArray<uint16> Buffer;
    Buffer.AddUninitialized(nFrames);

    int32 DataOffset = 0;
    for (int32 CurrentFrame = 0; CurrentFrame < Buffer.Num(); CurrentFrame++)
    {
        Buffer[CurrentFrame] = AudioData[DataOffset + NumChannels] * 32767; // 2^15, uint16

        DataOffset += (NumChannels * DownsampleRate);
    }

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
    TArray<uint16> Buffer;

    const int32 DownsampleRate = InSampleRate / gSamplesPerSec;
    const int32 nFrames = NFrames / DownsampleRate;
    Buffer.AddUninitialized(nFrames);

    int32 DataOffset = 0;
    for (int32 CurrentFrame = 0; CurrentFrame < nFrames; CurrentFrame++)
    {
        Buffer[CurrentFrame] = AudioData[DataOffset + NChannels];

        DataOffset += (NChannels * DownsampleRate);
    }

    Callback({(uint8*)Buffer.GetData(), Buffer.Num() * 2});
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
    const int32 DownsampleRate = SampleRate / gSamplesPerSec;
    const int32 nFrames = NumFrames / DownsampleRate;

    TArray<uint16> Buffer;
    Buffer.AddUninitialized(nFrames);

    int32 DataOffset = 0;
    for (int32 CurrentFrame = 0; CurrentFrame < Buffer.Num(); CurrentFrame++)
    {
        Buffer[CurrentFrame] = AudioData[DataOffset + NumChannels] * 32767; // 2^15, uint16

        DataOffset += (NumChannels * DownsampleRate);
    }

    Callback({ (uint8*)Buffer.GetData(), Buffer.Num() * 2 });
}
