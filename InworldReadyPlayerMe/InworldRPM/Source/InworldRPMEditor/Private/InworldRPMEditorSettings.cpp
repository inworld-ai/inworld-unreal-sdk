/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldRPMEditorSettings.h"
#include "Animation/AnimBlueprint.h"

#include "InworldCharacterComponent.h"
#include "InworldCharacterPlaybackHistory.h"
#include "InworldCharacterPlaybackTrigger.h"
#include "InworldCharacterAudioComponent.h"

UInworldRPMEditorSettings::UInworldRPMEditorSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	RPMAnimBP = "/InworldRPM/ABP_RPM.ABP_RPM";
	RPMSkeleton = "/InworldRPM/RPM_Mixamo_Skeleton.RPM_Mixamo_Skeleton";

	InworldCharacterComponent = UInworldCharacterComponent::StaticClass();
	CharacterPlaybacks = { UInworldCharacterPlaybackTrigger::StaticClass() };
	OtherCharacterComponents = { UInworldCharacterAudioComponent::StaticClass() };
}
