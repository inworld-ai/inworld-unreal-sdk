// Copyright 2023 Theai, Inc. (DBA Inworld) All Rights Reserved.


#include "InworldAudio2FaceSubsystem.h"
#include "InworldAudio2Face.h"

UInworldAudio2FaceSubsystem::UInworldAudio2FaceSubsystem()
{
	Audio2Face.OnInworldAudio2FaceAnimDataHeader.BindLambda(
		[this](const FAudio2FaceAnimDataHeader& Header)
		{
			OnInworldAudio2FaceAnimDataHeader.Broadcast(Header);
		});
	Audio2Face.OnInworldAudio2FaceAnimDataContent.BindLambda(
		[this](const FAudio2FaceAnimDataContent& Content)
		{
			OnInworldAudio2FaceAnimDataContent.Broadcast(Content);
		});
}

void UInworldAudio2FaceSubsystem::Start(const FString& URL)
{
	Audio2Face.Start(URL);
}

void UInworldAudio2FaceSubsystem::Stop()
{
	Audio2Face.Stop();
}

void UInworldAudio2FaceSubsystem::Update()
{
	Audio2Face.Update();
}

void UInworldAudio2FaceSubsystem::SendAudio(const TArray<uint8>& Audio)
{
	Audio2Face.SendAudio(Audio);
}

void UInworldAudio2FaceSubsystem::EndAudio()
{
	Audio2Face.EndAudio();
}
