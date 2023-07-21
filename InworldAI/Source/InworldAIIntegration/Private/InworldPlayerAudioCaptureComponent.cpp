/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldPlayerAudioCaptureComponent.h"
#include "InworldPlayerComponent.h"
#include "AudioMixerDevice.h"
#include "AudioMixerSubmix.h"
#include "InworldApi.h"
#include "InworldAIPlatformModule.h"
#include "NDK/Utils/Log.h"
#include <Net/UnrealNetwork.h>
#include <GameFramework/PlayerController.h>

constexpr uint32 gSamplesPerSec = 16000;

UInworldPlayerAudioCaptureComponent::UInworldPlayerAudioCaptureComponent()
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

    if (IsLocallyControlled())
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
        FScopeLock InputScopedLock(&InputCriticalSection);
        FScopeLock OutputScopedLock(&OutputCriticalSection);

        constexpr int32 SampleSendSize = gSamplesPerSec / 10; // 0.1s of data per send
        while (InputBuffer.Num() > SampleSendSize && (!bEnableAEC || OutputBuffer.Num() > SampleSendSize))
        {
            FPlayerVoiceCaptureInfoRep VoiceCaptureInfoRep;
            VoiceCaptureInfoRep.MicSoundData.Append(InputBuffer.GetData(), SampleSendSize);
            FMemory::Memcpy(InputBuffer.GetData(), InputBuffer.GetData() + SampleSendSize, (InputBuffer.Num() - SampleSendSize) * sizeof(int16));
            InputBuffer.SetNum(InputBuffer.Num() - SampleSendSize);

            if (bEnableAEC)
            {
                VoiceCaptureInfoRep.OutputSoundData.Append(OutputBuffer.GetData(), SampleSendSize);
                FMemory::Memcpy(OutputBuffer.GetData(), OutputBuffer.GetData() + SampleSendSize, (OutputBuffer.Num() - SampleSendSize) * sizeof(int16));
                OutputBuffer.SetNum(OutputBuffer.Num() - SampleSendSize);
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

    if (AudioCapture.IsStreamOpen())
    {
        return;
    }

    Audio::FOnCaptureFunction OnCapture = [this](const float* AudioData, int32 NumFrames, int32 NumChannels, int32 SampleRate, double StreamTime, bool bOverFlow)
    {
        OnAudioCapture(AudioData, NumFrames, NumChannels, SampleRate, StreamTime, bOverFlow);
    };

    AudioCapture.OpenCaptureStream(AudioCaptureDeviceParams, MoveTemp(OnCapture), 1024);
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

    if (!AudioCapture.IsStreamOpen())
    {
        return;
    }

    AudioCapture.CloseStream();
}

void UInworldPlayerAudioCaptureComponent::StartStream()
{
    if (!IsInAudioThread())
    {
        FAudioThread::RunCommandOnAudioThread([this]()
        {
            StartStream();
        });
        return;
    }

    if (!AudioCapture.IsStreamOpen())
    {
        return;
    }

    if (bCapturingVoice)
    {
        return;
    }

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

    bCapturingVoice = true;

    AudioCapture.StartStream();
}

void UInworldPlayerAudioCaptureComponent::StopStream()
{
    if (!IsInAudioThread())
    {
        FAudioThread::RunCommandOnAudioThread([this]()
        {
            StopStream();
        });
        return;
    }

    if (!AudioCapture.IsStreamOpen())
    {
        return;
    }

    if (!bCapturingVoice)
    {
        return;
    }

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

    bCapturingVoice = false;

    AudioCapture.StopStream();

    FScopeLock InputScopedLock(&InputCriticalSection);
    FScopeLock OutputScopedLock(&OutputCriticalSection);
    InputBuffer.Empty();
    OutputBuffer.Empty();
}

void UInworldPlayerAudioCaptureComponent::OnAudioCapture(const float* AudioData, int32 NumFrames, int32 NumChannels, int32 SampleRate, double StreamTime, bool bOverFlow)
{
    if (!bCapturingVoice)
    {
        return;
    }

    FScopeLock ScopedLock(&InputCriticalSection);

    const int32 DownsampleRate = SampleRate / gSamplesPerSec;
    const int32 nFrames = NumFrames / DownsampleRate;
    const int32 BufferOffset = InputBuffer.Num();
    InputBuffer.AddUninitialized(nFrames);

    int32 DataOffset = 0;
    for (int32 CurrentFrame = 0; CurrentFrame < nFrames; CurrentFrame++)
    {
        InputBuffer[BufferOffset + CurrentFrame] = (AudioData[DataOffset + NumChannels] * VolumeMultiplier) * 32768; // 2^15, uint16

        DataOffset += (NumChannels * DownsampleRate);
    }
}

void UInworldPlayerAudioCaptureComponent::OnNewSubmixBuffer(const USoundSubmix* OwningSubmix, float* AudioData, int32 NumSamples, int32 NumChannels, const int32 SampleRate, double AudioClock)
{
    if (!bCapturingVoice)
    {
        return;
    }

    FScopeLock ScopedLock(&OutputCriticalSection);

    const int32 NumFrames = NumSamples / NumChannels;

    const int32 DownsampleRate = SampleRate / gSamplesPerSec;
    const int32 nFrames = NumFrames / DownsampleRate;
    const int32 BufferOffset = OutputBuffer.Num();
    OutputBuffer.AddUninitialized(nFrames);

    int32 DataOffset = 0;
    for (int32 CurrentFrame = 0; CurrentFrame < nFrames; CurrentFrame++)
    {
        float DownsampleSum = 0.f;
        for (int32 i = 0; i < DownsampleRate; ++i)
        {
            DownsampleSum += AudioData[DataOffset + (i * NumChannels)] / DownsampleRate;
        }

        OutputBuffer[BufferOffset + CurrentFrame] = DownsampleSum * 32768; // 2^15, uint16

        DataOffset += (NumChannels * DownsampleRate);
    }
}

void UInworldPlayerAudioCaptureComponent::Server_ProcessVoiceCaptureChunk_Implementation(FPlayerVoiceCaptureInfoRep PlayerVoiceCaptureInfo)
{
	if (bEnableAEC)
	{
        std::vector<int16_t> Input, Output;
		Inworld::Utils::DataArray16ToVec16(PlayerVoiceCaptureInfo.MicSoundData, Input);
		Inworld::Utils::DataArray16ToVec16(PlayerVoiceCaptureInfo.OutputSoundData, Output);
        PlayerComponent->SendAudioDataMessageWithAECToTarget(Input, Output);
	}
	else
	{
		std::string Input;
		Inworld::Utils::DataArray16ToString(PlayerVoiceCaptureInfo.MicSoundData, Input);
        PlayerComponent->SendAudioDataMessageToTarget(Input);
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
