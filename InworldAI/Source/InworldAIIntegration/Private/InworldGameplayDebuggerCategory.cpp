/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#if defined(WITH_GAMEPLAY_DEBUGGER) && WITH_GAMEPLAY_DEBUGGER

#include "InworldGameplayDebuggerCategory.h"
#include "InworldApi.h"
#include "InworldCharacterComponent.h"
#include "Engine.h"
#include "Engine/Font.h"
#include "InworldPlayerAudioCaptureComponent.h"
#include "InworldPlayerComponent.h"

#include "UObject/UObjectIterator.h"

FInworldGameplayDebuggerCategory::FInworldGameplayDebuggerCategory()
{
	SetDataPackReplication<FRepData>(&DataPack);
	bShowOnlyWithDebugActor = false;
}

TSharedRef<FGameplayDebuggerCategory> FInworldGameplayDebuggerCategory::MakeInstance()
{
	return MakeShareable(new FInworldGameplayDebuggerCategory());
}

void FInworldGameplayDebuggerCategory::CollectData(APlayerController* OwnerPC, AActor* DebugActor)
{
	DataPack.bInworldApiExists = false;
	DataPack.CharData.Empty();
	DataPack.PlayerData.Empty();

	auto* InworldApi = OwnerPC->GetWorld()->GetSubsystem<UInworldApiSubsystem>();
	if (!InworldApi)
	{
		return;
	}

	DataPack.bInworldApiExists = true;
	DataPack.SessionStatus = static_cast<uint8>(InworldApi->Client->GetConnectionState());
	InworldApi->Client->GetConnectionError(DataPack.SessionError, DataPack.ErrorCode);
	DataPack.SessionId = InworldApi->Client->GetSessionId();

	for (auto* Component : InworldApi->GetCharacterComponents())
	{
		auto* Comp = static_cast<UInworldCharacterComponent*>(Component);
		if (!Comp)
		{
			continue;
		}

		auto& Data = DataPack.CharData.Emplace_GetRef();
		auto* Actor = Comp->GetComponentOwner();
		Data.OverheadLocation = Actor->GetActorLocation() + FVector(0, 0, Actor->GetSimpleCollisionHalfHeight());
		Data.bIsInteracting = Comp->IsInteractingWithPlayer();
		Data.GivenName = Comp->GetGivenName();
		Data.AgentId = Comp->GetAgentId();
		Data.CurrentMessage = Comp->GetCurrentMessage() ? Comp->GetCurrentMessage()->ToDebugString() : TEXT("");
		Data.MessageQueueEntries = Comp->MessageQueue->PendingMessageEntries.Num();
		Data.EmotionalBehavior = static_cast<uint8>(Comp->GetEmotionalBehavior());
		Data.EmotionStrength = static_cast<uint8>(Comp->GetEmotionStrength());
		Data.bPendingRepAudioEvent = !Comp->PendingRepAudioEvents.IsEmpty();
	}

	for (TObjectIterator<UInworldPlayerAudioCaptureComponent> Itr; Itr; ++Itr)
	{
		auto* AudioComp = *Itr;
		auto* PlayerComp = AudioComp->PlayerComponent.Get();

		if (!PlayerComp)
		{
			continue;
		}

		auto& Data = DataPack.PlayerData.Emplace_GetRef();
		//TODO: support multi agent
		Data.TargetCharacterAgentId = PlayerComp->Targets.IsEmpty() ? "" : PlayerComp->Targets[0].AgentId;
		Data.bServerCapturingVoice = AudioComp->bServerCapturingVoice;
	}
}

void FInworldGameplayDebuggerCategory::DrawData(APlayerController* OwnerPC, FGameplayDebuggerCanvasContext& CanvasContext)
{
	CanvasContext.Printf(TEXT("Inworld subsystem:   %s"), DataPack.bInworldApiExists ? TEXT("{green}Found") : TEXT("{red}Not found"));
	if (!DataPack.bInworldApiExists)
	{
		return;
	}

	static TMap<uint8, FString> StatusMap = 
	{
		{0, "{white}Idle"},
		{1, "{yellow}Connecting"},
		{2, "{green}Connected"},
		{3, "{red}Failed"},
		{4, "{yellow}Paused"},
		{5, "{red}Disconnected"},
		{6, "{yellow}Reconnecting"}
	};
	FString* StatusText = StatusMap.Find(DataPack.SessionStatus);
	if (StatusText)
	{
		CanvasContext.Printf(TEXT("Session status:   %s"), **StatusText);
	}
	if (!DataPack.SessionError.IsEmpty())
	{
		CanvasContext.Printf(TEXT("Error:   {red}%s"), *DataPack.SessionError);
		CanvasContext.Printf(TEXT("Error code:   {red}%d"), DataPack.ErrorCode);
	}
	CanvasContext.Printf(TEXT("Session Id:   {yellow}%s"), *DataPack.SessionId);

	int32 PlayerIdx = 0;
	for (auto& Data : DataPack.PlayerData)
	{
		CanvasContext.Printf(TEXT("[Player   %d]"), PlayerIdx++);
		CanvasContext.Printf(TEXT("Target Agent Id:   {yellow}%s"), *Data.TargetCharacterAgentId);
		CanvasContext.Printf(TEXT("Capturing voice:   %s"), Data.bServerCapturingVoice ? TEXT("{green}true") : TEXT("{yellow}false"));
	}

	static TMap<uint8, FString> EmotionalBehaviorMap =
	{
		{0, "NEUTRAL"},
		{1, "DISGUST"},
		{2, "CONTEMPT"},
		{3, "BELLIGERENCE"},
		{4, "DOMINEERING"},
		{5, "CRITICISM"},
		{6, "ANGER"},
		{7, "TENSION"},
		{8, "TENSE_HUMOR"},
		{9, "DEFENSIVENESS"},
		{10, "WHINING"},
		{11, "SADNESS"},
		{12, "STONEWALLING"},
		{13, "INTEREST"},
		{14, "VALIDATION"},
		{15, "AFFECTION"},
		{16, "HUMOR"},
		{17, "SURPRISE"},
		{18, "JOY"},
	};

	static TMap<uint8, FString> EmotionStrengthsMap =
	{
		{ 0, "UNSPECIFIED" },
		{ 1, "WEAK" },
		{ 2, "CONTEMPT" },
		{ 3, "STRONG" },
		{ 4, "NORMAL" },
	};

	FGameplayDebuggerCanvasContext OverheadContext(CanvasContext);
	OverheadContext.Font = GEngine->GetTinyFont();
	OverheadContext.FontRenderInfo.bEnableShadow = true;
	for (auto& Data : DataPack.CharData)
	{
		if (OverheadContext.IsLocationVisible(Data.OverheadLocation))
		{
			const FVector2D ScreenLoc = OverheadContext.ProjectLocation(Data.OverheadLocation);
			FString EmotionalBehavior = TEXT("None");
			if (auto* Val = EmotionalBehaviorMap.Find(Data.EmotionalBehavior))
			{
				EmotionalBehavior = *Val;
			}
			FString EmotionalStrength = TEXT("None");
			if (auto* Val = EmotionStrengthsMap.Find(Data.EmotionStrength))
			{
				EmotionalStrength = *Val;
			}
			FString Text = FString::Printf(TEXT("{white}Name: {yellow}%s\n{white}Agent Id: {yellow}%s\n{white}Interacting: %s\n{white}Current Message: {yellow}%s\n{white}Message Queue Entries: {yellow}%d\n{white}Pending Rep Audio Event: {yellow}%s\n{white}Emotional Behavior: {yellow}%s\n{white}Emotional Strength: {yellow}%s"), 
				*Data.GivenName, 
				*Data.AgentId, 
				Data.bIsInteracting ? TEXT("{green}true") : TEXT("{yellow}false"),
				*Data.CurrentMessage,
				Data.MessageQueueEntries,
				Data.bPendingRepAudioEvent ? TEXT("true") : TEXT("false"),
				*EmotionalBehavior,
				*EmotionalStrength);

			float SizeX = 0.0f, SizeY = 0.0f;
			OverheadContext.MeasureString(Text, SizeX, SizeY);
			OverheadContext.PrintAt(ScreenLoc.X - (SizeX * 0.5f), ScreenLoc.Y - (SizeY * 1.2f), Text);
		}
	}
}

void FInworldGameplayDebuggerCategory::FRepData::Serialize(FArchive& Ar)
{
	{
		int32 Num = PlayerData.Num();
		Ar << Num;
		if (Ar.IsLoading())
		{
			PlayerData.SetNum(Num);
		}
		for (int32 i = 0; i < Num; ++i)
		{
			PlayerData[i].Serialize(Ar);
		}
	}
	{
		int32 Num = CharData.Num();
		Ar << Num;
		if (Ar.IsLoading())
		{
			CharData.SetNum(Num);
		}
		for (int32 i = 0; i < Num; ++i)
		{
			CharData[i].Serialize(Ar);
		}
	}

	Ar << bInworldApiExists;
	Ar << SessionStatus;
	Ar << SessionError;
	Ar << SessionId;
}

void FInworldGameplayDebuggerCategory::FCharRepData::Serialize(FArchive& Ar)
{
	Ar << GivenName;
	Ar << AgentId;
	Ar << OverheadLocation;
	Ar << bIsInteracting;
	Ar << CurrentMessage;
	Ar << MessageQueueEntries;
	Ar << bPendingRepAudioEvent;
	Ar << EmotionalBehavior;
	Ar << EmotionStrength;
}

void FInworldGameplayDebuggerCategory::FPlayerRepData::Serialize(FArchive& Ar)
{
	Ar << TargetCharacterAgentId;
	Ar << bServerCapturingVoice;
}

#endif // defined(WITH_GAMEPLAY_DEBUGGER) && WITH_GAMEPLAY_DEBUGGER
