/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */


#include "InworldMetahumanEditorSettings.h"
#include "UObject/ConstructorHelpers.h"
#include "InworldCharacterPlaybackAudio.h"
#include "InworldCharacterPlaybackHistory.h"
#include "InworldCharacterPlaybackTrigger.h"
#include "Components/AudioComponent.h"

UInworldMetahumanEditorSettings::UInworldMetahumanEditorSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	MetahumanAnimBP = "/InworldMetahuman/ABP_Metahuman.ABP_Metahuman";
	MetahumanBody = "/Game/MetaHumans/Common/Female/Medium/NormalWeight/Body/metahuman_base_skel.metahuman_base_skel";

	MetahumanFaceAnimBP = "/InworldMetahuman/ABP_Metahuman_Face.ABP_Metahuman_Face";
	MetahumanFace = "/Game/MetaHumans/Common/Face/Face_Archetype_Skeleton.Face_Archetype_Skeleton";

	InworldCharacterComponent = UInworldCharacterComponent::StaticClass();
	CharacterPlaybacks = { UInworldCharacterPlaybackAudio::StaticClass(), UInworldCharacterPlaybackHistory::StaticClass(), UInworldCharacterPlaybackTrigger::StaticClass() };
	OtherCharacterComponents = { UAudioComponent::StaticClass() };
}
