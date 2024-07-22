/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */


#include "InworldCharacterA2FComponent.h"
#include "InworldCharacterComponent.h"
#include "InworldBlueprintFunctionLibrary.h"
#include "InworldAIAudio2FaceModule.h"
#include "AudioResampler.h"
#include "Sound/SoundWave.h"
#include "Sound/SoundWaveProcedural.h"
#include "TimerManager.h"

UInworldCharacterA2FComponent::UInworldCharacterA2FComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UInworldCharacterA2FComponent::BeginPlay()
{
	Super::BeginPlay();

	int32 OutVal;
	if (FParse::Value(FCommandLine::Get(), TEXT("a2fwaitbuffer="), OutVal))
	{
		MinPacketsToStart = OutVal;
	}

	SoundStreaming = NewObject<USoundWaveProcedural>();
	SoundStreaming->SetSampleRate(22050);
	SoundStreaming->NumChannels = 1;
	SoundStreaming->Duration = INDEFINITELY_LOOPING_DURATION;
	SoundStreaming->SoundGroup = SOUNDGROUP_Voice;
	SoundStreaming->bLooping = false;

	SoundStreaming->OnSoundWaveProceduralUnderflow = FOnSoundWaveProceduralUnderflow::CreateUObject(this, &UInworldCharacterA2FComponent::GenerateData);

	SetSound(SoundStreaming);

	CharacterComponent = Cast<UInworldCharacterComponent>(GetOwner()->GetComponentByClass(UInworldCharacterComponent::StaticClass()));
	if (CharacterComponent.IsValid())
	{
		CharacterComponent->OnUtterance.AddDynamic(this, &UInworldCharacterA2FComponent::OnCharacterUtterance);
		CharacterComponent->OnUtteranceInterrupt.AddDynamic(this, &UInworldCharacterA2FComponent::OnCharacterUtteranceInterrupt);
	}
}

void UInworldCharacterA2FComponent::EndPlay(EEndPlayReason::Type Reason)
{
	Super::EndPlay(Reason);

	SoundStreaming->ResetAudio();
	OnUtteranceInterrupted.Broadcast();
	OnUtteranceStopped.Broadcast();
	CharacterComponent->ClearMessageQueueLock(CharacterMessageQueueLockHandle);
}

void UInworldCharacterA2FComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	FScopeLock ScopeLock(&QueueLock);
	if (bIsActive)
	{
		if (!IsActive())
		{
			Play();
		}
	}
}

void UInworldCharacterA2FComponent::OnCharacterUtterance(const FCharacterMessageUtterance& Message)
{
	UtteranceId = Message.UtteranceId;
	UE_LOG(LogInworldAIAudio2Face, Log, TEXT("A2F OnCharacterUtterance: Processing for %s SoundData: %d"), *Message.UtteranceId, Message.SoundData.Num());

	if (Message.SoundData.Num() > 0 && Message.bAudioFinal)
	{
		UE_LOG(LogInworldAIAudio2Face, Log, TEXT("A2F OnCharacterUtterance: %s has locked the Queue"), *Message.UtteranceId);

		A2FData = Message.A2FData;
		A2FDataUpdateHandle = A2FData->OnCharacterMessageUtteranceA2FDataUpdate.AddUObject(this, &UInworldCharacterA2FComponent::OnCharacterMessageUtteranceA2FDataUpdate);

		FScopeLock ScopeLock(&QueueLock);
		AudioToPlay.Empty();
		AnimsToPlay.Empty();

		GotPackets = 0;
		bHasStartedProcessingAudio = false;
		bIsActive = true;

		OnCharacterMessageUtteranceA2FDataUpdate();

		CharacterComponent->MakeMessageQueueLock(CharacterMessageQueueLockHandle);

		if (!IsActive())
		{
			Play();
		}
	}
	else
	{
		UE_LOG(LogInworldAIAudio2Face, Log, TEXT("A2F OnCharacterUtterance: %s SKIP SKIP SKIP"), *Message.UtteranceId);
	}
}

void UInworldCharacterA2FComponent::OnCharacterUtteranceInterrupt(const FCharacterMessageUtterance& Message)
{
	FScopeLock ScopeLock(&QueueLock);

	if (A2FData)
	{
		A2FData->OnCharacterMessageUtteranceA2FDataUpdate.Remove(A2FDataUpdateHandle);
	}

	AudioToPlay.Empty();
	AnimsToPlay.Empty();

	GotPackets = 0;
	bHasStartedProcessingAudio = false;
	bIsActive = false;

	SoundStreaming->ResetAudio();
	OnUtteranceInterrupted.Broadcast();
	OnUtteranceStopped.Broadcast();
	UE_LOG(LogInworldAIAudio2Face, Log, TEXT("A2F OnCharacterUtterance: %s has unlocked the Queue (INTERRUPT)"), *UtteranceId);

	CharacterComponent->ClearMessageQueueLock(CharacterMessageQueueLockHandle);
}

void UInworldCharacterA2FComponent::OnCharacterMessageUtteranceA2FDataUpdate()
{
	FScopeLock ScopeLock(&QueueLock);

	while (!A2FData->PendingAudio.IsEmpty())
	{
		TArray<uint8> Audio;
		A2FData->PendingAudio.Dequeue(Audio);
		AudioToPlay.Enqueue(Audio);
		GotPackets++;
	}
	while (!A2FData->PendingBlendShapeMap.IsEmpty())
	{
		TMap<FName, float> BlendShapeMap;
		A2FData->PendingBlendShapeMap.Dequeue(BlendShapeMap);
		AnimsToPlay.Enqueue(BlendShapeMap);
	}

	if (!IsActive())
	{
		Play();
	}
}

void UInworldCharacterA2FComponent::GenerateData(USoundWaveProcedural* InProceduralWave, int32 SamplesRequired)
{
	if (A2FData == nullptr)
	{
		return;
	}
	if (!bIsActive)
	{
		return;
	}
	if (!bHasStartedProcessingAudio && !A2FData->bIsDone && GotPackets < MinPacketsToStart)
	{
		return;
	}

	FScopeLock ScopeLock(&QueueLock);
	if (!bHasStartedProcessingAudio)
	{
		UE_LOG(LogInworldAIAudio2Face, Log, TEXT("A2F OnCharacterUtterance: %s is starting to play, has gotten %d packets"), *UtteranceId, GotPackets);
	}
	bHasStartedProcessingAudio = true;

	TArray<uint8> ToQueue;
	if (AudioToPlay.Dequeue(ToQueue))
	{
		InProceduralWave->QueueAudio(ToQueue.GetData(), ToQueue.Num());
		TMap<FName, float> AnimToPlay;
		AnimsToPlay.Dequeue(AnimToPlay);
		AsyncTask(ENamedThreads::GameThread, [this, AnimToPlay]()
			{
				if (OnUtteranceStarted.IsBound())
				{
					OnUtteranceStarted.Broadcast();
				}
				FA2FBlendShapeData Data;
				Data.Map = AnimToPlay;
				OnInworldAudio2FaceBlendShapeUpdate.Broadcast(Data);
			});
	}
	else if (A2FData->bIsDone)
	{
		if (A2FData)
		{
			A2FData->OnCharacterMessageUtteranceA2FDataUpdate.Remove(A2FDataUpdateHandle);
		}

		A2FData = nullptr;

		AudioToPlay.Empty();
		AnimsToPlay.Empty();

		GotPackets = 0;
		bHasStartedProcessingAudio = false;
		bIsActive = false;
		SoundStreaming->ResetAudio();

		UE_LOG(LogInworldAIAudio2Face, Log, TEXT("A2F OnCharacterUtterance: %s has unlocked the Queue (OUT OF DATA, IS DONE)"), *UtteranceId);

		AsyncTask(ENamedThreads::GameThread, [this]()
			{
				FScopeLock ScopeLock(&QueueLock);
				OnUtteranceStopped.Broadcast();
				CharacterComponent->ClearMessageQueueLock(CharacterMessageQueueLockHandle);
			});
	}
}
