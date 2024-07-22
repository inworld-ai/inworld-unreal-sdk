// Copyright 2023 Theai, Inc. (DBA Inworld) All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "InworldAudio2Face.generated.h"

namespace Inworld
{
	class FAudio2Face;
}

USTRUCT(BlueprintType)
struct INWORLDAIAUDIO2FACE_API FAudio2FaceAnimDataHeader
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	bool Success = false;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	FString Message;
};

USTRUCT(BlueprintType)
struct INWORLDAIAUDIO2FACE_API FAudio2FaceAnimDataContent
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	FString USDA;

	TMap<FString, TArray<uint8>> Files;
};

USTRUCT()
struct INWORLDAIAUDIO2FACE_API FInworldAudio2Face
{
public:
	GENERATED_BODY()

	void Start(const FString& URL);
	void Stop();

	void SendAudio(const TArray<uint8>& AudioData);
	void EndAudio();

	void Update();

	DECLARE_DELEGATE_OneParam(FOnInworldAudio2FaceAnimDataHeader, const FAudio2FaceAnimDataHeader&);
	FOnInworldAudio2FaceAnimDataHeader OnInworldAudio2FaceAnimDataHeader;

	DECLARE_DELEGATE_OneParam(FOnInworldAudio2FaceAnimDataContent, const FAudio2FaceAnimDataContent&);
	FOnInworldAudio2FaceAnimDataContent OnInworldAudio2FaceAnimDataContent;

private:
	using SendToSendMethod = void (Inworld::FAudio2Face::*)(const std::string& audio);
	void SendToSend(SendToSendMethod Method);

	bool bHasSentAudio = false;
	TOptional<TArray<uint8>> ToSend;
	TSharedPtr<Inworld::FAudio2Face> InworldAudio2Face;
};

UCLASS()
class INWORLDAIAUDIO2FACE_API UInworldA2FBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintPure, Category = "Audio2Face")
	static bool GetFileFromA2FContent(const FAudio2FaceAnimDataContent& A2FContent, const FString& File, TArray<uint8>& OutData)
	{
		if (A2FContent.Files.Contains(File))
		{
			OutData = A2FContent.Files[File];
			return true;
		}
		return false;
	}

	UFUNCTION(BlueprintPure, Category = "Audio2Face")
	static void GetFileNamesFromA2FContent(const FAudio2FaceAnimDataContent& A2FContent, TArray<FString>& OutFileNames)
	{
		A2FContent.Files.GetKeys(OutFileNames);
	}
};
