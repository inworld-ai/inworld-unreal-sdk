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

#include "IPixelStreamingModule.h"
#include "IPixelStreamingStreamer.h"

#include <Net/UnrealNetwork.h>
#include <GameFramework/PlayerController.h>

constexpr uint32 gSamplesPerSec = 16000;

UInworldPlayerAudioCaptureComponent::UInworldPlayerAudioCaptureComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , AudioSink(nullptr)
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
        OpenStream();
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
        StopStream();
        CloseStream();
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

        constexpr int32 SampleSendSize = gSamplesPerSec / 10; // 0.1s of data per send
        while (InputBuffer.Data.Num() > SampleSendSize && (!bEnableAEC || OutputBuffer.Data.Num() > SampleSendSize))
        {
            FPlayerVoiceCaptureInfoRep VoiceCaptureInfoRep;
            VoiceCaptureInfoRep.MicSoundData.Append((uint8*)InputBuffer.Data.GetData(), SampleSendSize * 2);
            FMemory::Memcpy(InputBuffer.Data.GetData(), InputBuffer.Data.GetData() + SampleSendSize, (InputBuffer.Data.Num() - SampleSendSize) * sizeof(int16));
            InputBuffer.Data.SetNum(InputBuffer.Data.Num() - SampleSendSize);

            if (bEnableAEC)
            {
                VoiceCaptureInfoRep.OutputSoundData.Append((uint8*)OutputBuffer.Data.GetData(), SampleSendSize * 2);
                FMemory::Memcpy(OutputBuffer.Data.GetData(), OutputBuffer.Data.GetData() + SampleSendSize, (OutputBuffer.Data.Num() - SampleSendSize) * sizeof(int16));
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
    if (bPixelStream)
    {
        return;
    }

    if (!IsInAudioThread())
    {
        FAudioThread::RunCommandOnAudioThread([this, DeviceId]()
        {
            SetCaptureDeviceById(DeviceId);
        });
        return;
    }

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

    const bool bWasCapturingVoice = bCapturingVoice;

    if (bWasCapturingVoice)
    {
        StopStream();
    }

    CloseStream();

    OpenStream();

    if (bWasCapturingVoice)
    {
        StartStream();
    }
}

void UInworldPlayerAudioCaptureComponent::OpenStream()
{
    if (!IsInAudioThread())
    {
        FAudioThread::RunCommandOnAudioThread([this]()
            {
                OpenStream();
            });
        return;
    }

    if (!bPixelStream)
    {
        if (!AudioCapture.IsStreamOpen())
        {
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 3
            Audio::FOnAudioCaptureFunction OnCapture = [this](const void* AudioData, int32 NumFrames, int32 NumChannels, int32 SampleRate, double StreamTime, bool bOverFlow)
                {
                    OnAudioCapture((float*)AudioData, NumFrames, NumChannels, SampleRate, InputBuffer);
                };

            AudioCapture.OpenAudioCaptureStream(AudioCaptureDeviceParams, MoveTemp(OnCapture), 1024);
#else
            Audio::FOnCaptureFunction OnCapture = [this](const float* AudioData, int32 NumFrames, int32 NumChannels, int32 SampleRate, double StreamTime, bool bOverFlow)
                {
                    OnAudioCapture(AudioData, NumFrames, NumChannels, SampleRate, InputBuffer);
                };

            AudioCapture.OpenCaptureStream(AudioCaptureDeviceParams, MoveTemp(OnCapture), 1024);
#endif
        }
    }
}

void UInworldPlayerAudioCaptureComponent::CloseStream()
{
    if (!IsInAudioThread())
    {
        FAudioThread::RunCommandOnAudioThread([this]()
            {
                CloseStream();
            });
        return;
    }

    if (!bPixelStream)
    {
        if (AudioCapture.IsStreamOpen())
        {
            AudioCapture.CloseStream();
        }
    }
}

void UInworldPlayerAudioCaptureComponent::StartStream()
{
    if (bCapturingVoice)
    {
        return;
    }

    if (!IsInAudioThread())
    {
        FAudioThread::RunCommandOnAudioThread([this]()
            {
                StartStream();
            });
        return;
    }

    if (bPixelStream)
    {
        IPixelStreamingModule& PixelStreamingModule = IPixelStreamingModule::Get();
        if (!PixelStreamingModule.IsReady())
        {
            return;
        }

        TSharedPtr<IPixelStreamingStreamer> Streamer = PixelStreamingModule.FindStreamer(PixelStreamingModule.GetDefaultStreamerID());
        if (!Streamer)
        {
            return;
        }

        IPixelStreamingAudioSink* CandidateSink = Streamer->GetUnlistenedAudioSink();

        if (CandidateSink == nullptr)
        {
            return;
        }

        AudioSink = CandidateSink;
        AudioSink->AddAudioConsumer(this);
    }
    else
    {
        if (!AudioCapture.IsStreamOpen())
        {
            return;
        }
        AudioCapture.StartStream();
    }

    bCapturingVoice = true;

    if (bEnableAEC)
    {
        Audio::FMixerDevice* MixerDevice = static_cast<Audio::FMixerDevice*>(GetWorld()->GetAudioDeviceRaw());
        if (MixerDevice)
        {
            Audio::FMixerSubmixPtr MasterSubmix = MixerDevice->GetMasterSubmix().Pin();
            if (MasterSubmix.IsValid())
            {
                MasterSubmix->RegisterBufferListener(this);
            }
        }
    }
}

void UInworldPlayerAudioCaptureComponent::StopStream()
{
    if (!bCapturingVoice)
    {
        return;
    }

    if (!IsInAudioThread())
    {
        FAudioThread::RunCommandOnAudioThread([this]()
            {
                StopStream();
            });
        return;
    }

    if (bPixelStream)
    {
        if (AudioSink)
        {
            AudioSink->RemoveAudioConsumer(this);
        }

        AudioSink = nullptr;
    }
    else
    {
        if (!AudioCapture.IsStreamOpen())
        {
            return;
        }
        AudioCapture.StopStream();
    }

    bCapturingVoice = false;

    if (bEnableAEC)
    {
        Audio::FMixerDevice* MixerDevice = static_cast<Audio::FMixerDevice*>(GetWorld()->GetAudioDeviceRaw());
        if (MixerDevice)
        {
            Audio::FMixerSubmixPtr MasterSubmix = MixerDevice->GetMasterSubmix().Pin();
            if (MasterSubmix.IsValid())
            {
                MasterSubmix->UnregisterBufferListener(this);
            }
        }
    }

    FScopeLock InputScopedLock(&InputBuffer.CriticalSection);
    FScopeLock OutputScopedLock(&OutputBuffer.CriticalSection);
    InputBuffer.Data.Empty();
    OutputBuffer.Data.Empty();
}

void UInworldPlayerAudioCaptureComponent::OnAudioCapture(const float* AudioData, int32 NumFrames, int32 NumChannels, int32 SampleRate, struct FInworldAudioBuffer& WriteBuffer)
{
    if (!bCapturingVoice)
    {
        return;
    }

    FScopeLock ScopedLock(&WriteBuffer.CriticalSection);

    const int32 DownsampleRate = SampleRate / gSamplesPerSec;
    const int32 nFrames = NumFrames / DownsampleRate;
    const int32 BufferOffset = WriteBuffer.Data.Num();
    WriteBuffer.Data.AddUninitialized(nFrames);

    int32 DataOffset = 0;
    for (int32 CurrentFrame = 0; CurrentFrame < nFrames; CurrentFrame++)
    {
        WriteBuffer.Data[BufferOffset + CurrentFrame] = (uint16)((AudioData[DataOffset + NumChannels] * VolumeMultiplier) * 32768); // 2^15, uint16

        DataOffset += (NumChannels * DownsampleRate);
    }
}

void UInworldPlayerAudioCaptureComponent::OnNewSubmixBuffer(const USoundSubmix* OwningSubmix, float* AudioData, int32 NumSamples, int32 NumChannels, const int32 SampleRate, double AudioClock)
{
    const int32 NumFrames = NumSamples / NumChannels;
    OnAudioCapture(AudioData, NumFrames, NumChannels, SampleRate, OutputBuffer);
}

void UInworldPlayerAudioCaptureComponent::ConsumeRawPCM(const int16_t* AudioData, int InSampleRate, size_t NChannels, size_t NFrames)
{
    TArray<float> fAudioData;

    const int32 DownsampleRate = InSampleRate / gSamplesPerSec;
    const int32 nFrames = NFrames / DownsampleRate;
    fAudioData.AddUninitialized(nFrames);

    int32 DataOffset = 0;
    for (int32 CurrentFrame = 0; CurrentFrame < nFrames; CurrentFrame++)
    {
        fAudioData[CurrentFrame] = (float)((AudioData[DataOffset + NChannels]) / 32768.f); // 2^15, uint16

        DataOffset += (NChannels * DownsampleRate);
    }

    OnAudioCapture(fAudioData.GetData(), nFrames, 1, gSamplesPerSec, InputBuffer);
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
        StartStream();
    }
    else
    {
        StopStream();
    }
}
