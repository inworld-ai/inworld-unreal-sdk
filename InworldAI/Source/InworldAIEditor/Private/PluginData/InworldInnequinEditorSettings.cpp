/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */


#include "InworldInnequinEditorSettings.h"
#include "UObject/ConstructorHelpers.h"

#include "InworldCharacterComponent.h"
#include "InworldCharacterPlaybackHistory.h"
#include "InworldCharacterPlaybackTrigger.h"
#include "InworldCharacterAudioComponent.h"

UInworldInnequinEditorSettings::UInworldInnequinEditorSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InnequinMesh = FSoftObjectPath{ TEXT("SkeletalMesh'/InworldInnequin/Mesh/SM_Innequin.SM_Innequin'") };
	InnequinABP = FSoftObjectPath{ TEXT("AnimBlueprint'/InworldInnequin/Animation/ABP_Innequin.ABP_Innequin'") };

	CharacterPlaybacks =
	{
		UInworldCharacterPlaybackTrigger::StaticClass(),
		LoadClass<UInworldCharacterPlayback>(nullptr, TEXT("BlueprintGeneratedClass'/InworldInnequin/Animation/InworldCharacterPlayback_InnequinAnimation.InworldCharacterPlayback_InnequinAnimation_C'"))
	};

	CharacterComponents =
	{
		LoadClass<UActorComponent>(nullptr, TEXT("BlueprintGeneratedClass'/InworldInnequin/BP_InnequinComponent.BP_InnequinComponent_C'")),
		LoadClass<UActorComponent>(nullptr, TEXT("Class'/Script/Paper2D.PaperFlipbookComponent'"))
	};
}
