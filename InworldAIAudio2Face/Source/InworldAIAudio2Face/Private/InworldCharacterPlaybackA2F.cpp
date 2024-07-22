// Copyright 2023 Theai, Inc. (DBA Inworld) All Rights Reserved.


#include "InworldCharacterPlaybackA2F.h"
#include "InworldCharacterComponent.h"
#include "InworldBlueprintFunctionLibrary.h"
#include "AudioResampler.h"
#include "Sound/SoundWave.h"
#include "Sound/SoundWaveProcedural.h"
#include <Components/AudioComponent.h>

//#include "USDWrappers/USDStage.h"

void UInworldCharacterPlaybackA2F::BeginPlay_Implementation()
{
	Super::BeginPlay_Implementation();

	{
		int32 OutVal;
		if (FParse::Value(FCommandLine::Get(), TEXT("minpackets="), OutVal))
		{
			MinPacketsToStart = OutVal;
		}
	}

	InworldAudio2Face.OnInworldAudio2FaceAnimDataHeader.BindUObject(this, &UInworldCharacterPlaybackA2F::OnInworldAudio2FaceAnimDataHeader);
	InworldAudio2Face.OnInworldAudio2FaceAnimDataContent.BindUObject(this, &UInworldCharacterPlaybackA2F::OnInworldAudio2FaceAnimDataContent);

	AudioComponent = Cast<UAudioComponent>(OwnerActor->GetComponentByClass(UAudioComponent::StaticClass()));

	if (ensureMsgf(AudioComponent.IsValid(), TEXT("UInworldCharacterPlaybackA2F owner doesn't contain AudioComponent")))
	{
		SoundStreaming = NewObject<USoundWaveProcedural>();
		SoundStreaming->SetSampleRate(16000);
		SoundStreaming->NumChannels = 1;
		SoundStreaming->Duration = INDEFINITELY_LOOPING_DURATION;
		SoundStreaming->SoundGroup = SOUNDGROUP_Voice;
		SoundStreaming->bLooping = false;

		SoundStreaming->OnSoundWaveProceduralUnderflow = FOnSoundWaveProceduralUnderflow::CreateUObject(this, &UInworldCharacterPlaybackA2F::GenerateData);

		AudioComponent->SetSound(SoundStreaming);
	}

	// For now, just run A2F connection over lifespan
	InworldAudio2Face.Start("104.197.72.238:52010");
}

void UInworldCharacterPlaybackA2F::EndPlay_Implementation()
{
	Super::EndPlay_Implementation();

	// For now, just run A2F connection over lifespan
	InworldAudio2Face.Stop();
}

void UInworldCharacterPlaybackA2F::Tick_Implementation(float DeltaTime)
{
	Super::Tick_Implementation(DeltaTime);

	// HACK: If we send all data immediately, we get error on Microservice - just send a packet once per tick.
	// We should be ticking at > 30 FPS, which is the rate of audio, so we shouldn't ever fall behind
	
//	static volatile int32 nbIterations = 2;
//	for (int i = 0; i < nbIterations; i++)
	/*
	if (!AudioToSend.IsEmpty())
	{
		TArray<uint8> ToSend;
		AudioToSend.Dequeue(ToSend);
		InworldAudio2Face.SendAudio(ToSend);

		if (AudioToSend.IsEmpty())
		{
			InworldAudio2Face.EndAudio();
		}
	}*/

	InworldAudio2Face.Update();
}

void UInworldCharacterPlaybackA2F::OnCharacterUtterance_Implementation(const FCharacterMessageUtterance& Message)
{
	if (Message.SoundData.Num() > 0 && Message.bAudioFinal)
	{
		FWaveModInfo WaveInfo;
		if (!WaveInfo.ReadWaveInfo(Message.SoundData.GetData(), Message.SoundData.Num()))
		{
			return;
		}

		const float Duration = *WaveInfo.pWaveDataSize / (*WaveInfo.pChannels * (*WaveInfo.pBitsPerSample / 8.f) * *WaveInfo.pSamplesPerSec);

		TArray<int16> Data = { (int16*)WaveInfo.SampleDataStart, (int32)WaveInfo.SampleDataSize / 2 };
		Audio::AlignedFloatBuffer InputBuffer;
		InputBuffer.SetNumUninitialized(Data.Num());
		for (int32 i = 0; i < Data.Num(); ++i)
		{
			InputBuffer[i] = ((float)Data[i]) / 32767.f; // 2^15, int16
		}

		Audio::FResamplingParameters ResamplerParams = {
			Audio::EResamplingMethod::Linear,
			1,
			(float)*WaveInfo.pSamplesPerSec,
			16000.f,
			InputBuffer
		};

		Audio::FAlignedFloatBuffer OutputBuffer;
		OutputBuffer.AddUninitialized(Audio::GetOutputBufferSize(ResamplerParams));

		Audio::FResamplerResults ResamplerResults;
		ResamplerResults.OutBuffer = &OutputBuffer;

		TArray<float> ResampledAudioData;
		if (Audio::Resample(ResamplerParams, ResamplerResults))
		{
			ResampledAudioData = { ResamplerResults.OutBuffer->GetData(), ResamplerResults.OutputFramesGenerated };

			const int32 Frames = Duration * 30.f; // 30 fps
			const int32 DataPerFrame = 533;
			for (int32 CurrentFrameStart = 0; CurrentFrameStart < ResampledAudioData.Num(); CurrentFrameStart += DataPerFrame)
			{
				const int32 CurrentFrameEnd = FMath::Clamp(CurrentFrameStart + DataPerFrame, 0, ResampledAudioData.Num() - 1);

				TArray<uint8> AudioData{ (uint8*)(ResampledAudioData.GetData() + CurrentFrameStart), (CurrentFrameEnd - CurrentFrameStart) * 4 };
				//AudioToSend.Enqueue(AudioData);
				InworldAudio2Face.SendAudio(AudioData);
			}

			InworldAudio2Face.EndAudio();

			RemainingAudio = ResampledAudioData.Num();

			GotPackets = 0;

			bIsActive = true;

			LockMessageQueue();
			OnUtteranceStarted.Broadcast(Message, GetOwner());
		}
	}
}

void UInworldCharacterPlaybackA2F::OnCharacterUtteranceInterrupt_Implementation(const FCharacterMessageUtterance& Message)
{
	FScopeLock ScopeLock(&QueueLock);
	SoundStreaming->ResetAudio();
	AudioToSend.Empty();
	AudioToPlay.Empty();
	AnimsToPlay.Empty();
	OnUtteranceInterrupted.Broadcast();
	OnUtteranceStopped.Broadcast();
	bIsActive = false;
	GotPackets = 0;
	UnlockMessageQueue();
}

void UInworldCharacterPlaybackA2F::OnInworldAudio2FaceAnimDataHeader(const FAudio2FaceAnimDataHeader& Header)
{
	// TODO: Need header data for anything?
}

void UInworldCharacterPlaybackA2F::OnInworldAudio2FaceAnimDataContent(const FAudio2FaceAnimDataContent& Content)
{
	TArray<FString> OutKeys;
	UInworldA2FBlueprintFunctionLibrary::GetFileNamesFromA2FContent(Content, OutKeys);
	if (OutKeys.Num() > 0)
	{
		GotPackets++;

		TArray<uint8> RawData;
		UInworldA2FBlueprintFunctionLibrary::GetFileFromA2FContent(Content, OutKeys[0], RawData);
		{
			FScopeLock ScopeLock(&QueueLock);

			TArray<float> FloatData{ (float*)(RawData.GetData() + 44), (RawData.Num() - 44) / 4 };
			TArray<int16> PCMData;
			PCMData.AddUninitialized(FloatData.Num());
			for (int32 i = 0; i < FloatData.Num(); ++i)
			{
				PCMData[i] = FloatData[i] * 32767;  // 2^15, int16
			}

			AudioToPlay.Enqueue({ (uint8*)PCMData.GetData(), PCMData.Num() * 2 });
			RemainingAudio -= FloatData.Num();
		}

		// TODO: Use UnrealUSDWrapper -> UsdStage.GetRootLater().ImportFromString() is unavailable(?)
		// This is not optimal, but quck hack to extract data
		const int32 BlendshapesBegin = Content.USDA.Find(FString("uniform token[] blendShapes = [")) + 31;
		const int32 BlendshapesEnd = Content.USDA.Find(FString("]"), ESearchCase::IgnoreCase, ESearchDir::FromStart, BlendshapesBegin);
		const FString Blendshapes = Content.USDA.Mid(BlendshapesBegin, BlendshapesEnd - BlendshapesBegin);
		TArray<FString> BlendKeys;
		Blendshapes.ParseIntoArray(BlendKeys, TEXT(","));
		for (FString& BlendKey : BlendKeys)
		{
			BlendKey = BlendKey.TrimQuotes();
		}

		const int32 BlendshapeWeightsBegin = Content.USDA.Find(FString("float[] blendShapeWeights = [")) + 29;
		const int32 BlendshapeWeightsEnd = Content.USDA.Find(FString("]"), ESearchCase::IgnoreCase, ESearchDir::FromStart, BlendshapeWeightsBegin);
		const FString BlendshapeWeights = Content.USDA.Mid(BlendshapeWeightsBegin, BlendshapeWeightsEnd - BlendshapeWeightsBegin);
		TArray<FString> BlendValues;
		BlendshapeWeights.ParseIntoArray(BlendValues, TEXT(","));

		TMap<FName, float> BlendshapeMap;
		for (int32 i = 0; i < BlendKeys.Num(); ++i)
		{
			BlendshapeMap.Add(
				FName(*BlendKeys[i]),
				FCString::Atof(*BlendValues[i])
			);
		}

		AnimsToPlay.Enqueue(BlendshapeMap);
	}

	if (!AudioComponent->IsActive())
	{
		AudioComponent->Play();
	}
}

void UInworldCharacterPlaybackA2F::GenerateData(USoundWaveProcedural* InProceduralWave, int32 SamplesRequired)
{
	if (!bIsActive)
	{
		return;
	}

	if (RemainingAudio > 0 && GotPackets < MinPacketsToStart)
	{
		return;
	}

	FScopeLock ScopeLock(&QueueLock);
	if (!AudioToPlay.IsEmpty())
	{
		TArray<uint8> ToQueue;
		AudioToPlay.Dequeue(ToQueue);
		InProceduralWave->QueueAudio(ToQueue.GetData(), ToQueue.Num());

		TMap<FName, float> ToPlay;
		AnimsToPlay.Dequeue(ToPlay);
		AsyncTask(ENamedThreads::GameThread, [this, ToPlay]()
			{
				FA2FBlendShapeData Data{ ToPlay };
				OnInworldAudio2FaceBlendShapeUpdate.Broadcast(Data);
			});
	}
	else if (RemainingAudio <= 0)
	{
		bIsActive = false;
		AsyncTask(ENamedThreads::GameThread, [this]()
		{
			OnUtteranceStopped.Broadcast();
			UnlockMessageQueue();
		});
	}
}