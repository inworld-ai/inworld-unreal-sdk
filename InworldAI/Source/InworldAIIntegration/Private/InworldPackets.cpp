/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldPackets.h"

void AppendToDebugString(FString& DbgStr, const FString& Str)
{
	DbgStr.Append(Str);
	DbgStr.Append(TEXT(". "));
}

FInworldAudioDataEvent::FInworldAudioDataEvent(const Inworld::AudioDataEvent& Event)
	: FInworldDataEvent(Event)
{
	PhonemeInfos.Reserve(Event.GetPhonemeInfos().size());
	for (const auto& Info : Event.GetPhonemeInfos())
	{
		PhonemeInfos.Add(FInworldPhonemeInfo(Info));
	}
}

void FInworldAudioDataEvent::ConvertToReplicatableEvents(const FInworldAudioDataEvent& Event, TArray<FInworldAudioDataEvent>& RepEvents)
{
	constexpr uint32 DataMaxSize = 32 * 1024;

	const uint32 NumArrays = Event.Chunk.size() / DataMaxSize + 1;
	RepEvents.SetNum(NumArrays);
	TArray<std::string*> Chunks;
	Chunks.Reserve(NumArrays);

	for (auto& E : RepEvents)
	{
		E.PacketId = Event.PacketId;
		E.Routing = Event.Routing;
		E.bFinal = false;

		Chunks.Add(&E.Chunk);
	}
	Inworld::Utils::StringToArrayStrings(Event.Chunk, Chunks, DataMaxSize);

	auto& FinalEvent = RepEvents.Last();
	FinalEvent.bFinal = true;
	FinalEvent.PhonemeInfos = Event.PhonemeInfos;
}

template<typename T>
void SerializeValue(FMemoryArchive& Ar, T& Val)
{
	Ar.Serialize(&Val, sizeof(Val));
}

template<typename T>
void SerializeArray(FMemoryArchive& Ar, TArray<T>& Array)
{
	int32 Size = Array.Num();
	SerializeValue<int32>(Ar, Size);

	Array.SetNumUninitialized(Size);

	for (T& Val : Array)
	{
		SerializeValue<T>(Ar, Val);
	}
}

template<typename T>
void SerializeStructArray(FMemoryArchive& Ar, TArray<T>& Array)
{
	int32 Size = Array.Num();
	SerializeValue<int32>(Ar, Size);

	Array.SetNum(Size);

	for (T& Val : Array)
	{
		Val.Serialize(Ar);
	}
}

static void SerializeString(FMemoryArchive& Ar, std::string& Str)
{
	int32 Size = Str.size();
	SerializeValue<int32>(Ar, Size);

	Str.resize(Size);

	Ar.Serialize((void*)Str.data(), Size);
}

static void SerializeString(FMemoryArchive& Ar, FString& Str)
{
	int32 Size = Str.Len();
	SerializeValue<int32>(Ar, Size);

	if (Ar.IsLoading())
	{
		Str = FString(Size, TEXT("0"));
	}

	Ar.Serialize((void*)Str.GetCharArray().GetData(), sizeof(TCHAR) * Size);
}

void FInworldActor::Serialize(FMemoryArchive& Ar)
{
	uint8 T = static_cast<uint8>(Type);
	SerializeValue<uint8>(Ar, T);
	Type = static_cast<EInworldActorType>(T);

	SerializeString(Ar, Name);
}

void FInworldActor::AppendDebugString(FString& Str) const
{
	switch (Type)
	{
	case EInworldActorType::UNKNOWN:
		AppendToDebugString(Str, TEXT("UNKNOWN"));
		break;
	case EInworldActorType::PLAYER:
		AppendToDebugString(Str, TEXT("PLAYER"));
		break;
	case EInworldActorType::AGENT:
		AppendToDebugString(Str, TEXT("AGENT"));
		break;
	default:
		break;
	}
	AppendToDebugString(Str, Name);
}

void FInworldRouting::Serialize(FMemoryArchive& Ar)
{
	Source.Serialize(Ar);
	Target.Serialize(Ar);
}

void FInworldRouting::AppendDebugString(FString& Str) const
{
	AppendToDebugString(Str, TEXT("Source"));
	Source.AppendDebugString(Str);

	AppendToDebugString(Str, TEXT("Target"));
	Target.AppendDebugString(Str);
}

void FInworldPacketId::Serialize(FMemoryArchive& Ar)
{
	SerializeString(Ar, UID);
	SerializeString(Ar, UtteranceId);
	SerializeString(Ar, InteractionId);
}

void FInworldPacketId::AppendDebugString(FString& Str) const
{
	AppendToDebugString(Str, UID);
	AppendToDebugString(Str, UtteranceId);
	AppendToDebugString(Str, InteractionId);
}

void FInworldPacket::Serialize(FMemoryArchive& Ar)
{
	PacketId.Serialize(Ar);
	Routing.Serialize(Ar);
}

FString FInworldPacket::ToDebugString() const
{
	FString Str;
	AppendDebugString(Str);
	Routing.AppendDebugString(Str);
	PacketId.AppendDebugString(Str);
	return Str;
}

void FInworldDataEvent::Serialize(FMemoryArchive& Ar)
{
	FInworldPacket::Serialize(Ar);

	SerializeString(Ar, Chunk);
}

void FInworldDataEvent::AppendDebugString(FString& Str) const
{
	AppendToDebugString(Str, TEXT("Data"));
	AppendToDebugString(Str, FString::FromInt(Chunk.size()));
}

void FInworldAudioDataEvent::Serialize(FMemoryArchive& Ar)
{
	FInworldDataEvent::Serialize(Ar);

	SerializeStructArray<FInworldPhonemeInfo>(Ar, PhonemeInfos);
	SerializeValue<bool>(Ar, bFinal);
}

void FInworldAudioDataEvent::AppendDebugString(FString& Str) const
{
	FInworldDataEvent::AppendDebugString(Str);

	AppendToDebugString(Str, TEXT("Audio"));
	AppendToDebugString(Str, FString::FromInt(PhonemeInfos.Num()));
	AppendToDebugString(Str, bFinal ? TEXT("Final") : TEXT("Not final"));
}

void FInworldPhonemeInfo::Serialize(FMemoryArchive& Ar)
{
	SerializeString(Ar, Code);
	SerializeValue<float>(Ar, Timestamp);
}

void FInworldTextEvent::AppendDebugString(FString& Str) const
{
	AppendToDebugString(Str, TEXT("Text"));
	AppendToDebugString(Str, Text);
	AppendToDebugString(Str, Final ? TEXT("Final") : TEXT("Not final"));
}

void FInworldSilenceEvent::AppendDebugString(FString& Str) const
{
	AppendToDebugString(Str, TEXT("Silence"));
	AppendToDebugString(Str, FString::SanitizeFloat(Duration));
}

void FInworldControlEvent::AppendDebugString(FString& Str) const
{
	AppendToDebugString(Str, TEXT("Control"));
	AppendToDebugString(Str, FString::FromInt(static_cast<int32>(Action)));
}

void FInworldEmotionEvent::AppendDebugString(FString& Str) const
{
	AppendToDebugString(Str, TEXT("Emotion"));
	AppendToDebugString(Str, FString::FromInt(static_cast<int32>(Behavior)));
	AppendToDebugString(Str, FString::FromInt(static_cast<int32>(Strength)));
}

void FInworldCustomEvent::AppendDebugString(FString& Str) const
{
	AppendToDebugString(Str, TEXT("Custom"));
	AppendToDebugString(Str, Name);
}

void FInworldChangeSceneEvent::AppendDebugString(FString& Str) const
{
	AppendToDebugString(Str, TEXT("ChangeScene"));
	for (auto& Agent : AgentInfos)
	{
		AppendToDebugString(Str, UTF8_TO_TCHAR(Agent.GivenName.c_str()));
	}
}
