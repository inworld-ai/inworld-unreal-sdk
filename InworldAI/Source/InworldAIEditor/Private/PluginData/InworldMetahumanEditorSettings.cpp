/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */


#include "InworldMetahumanEditorSettings.h"
#include "UObject/ConstructorHelpers.h"

#include "InworldCharacterComponent.h"
#include "InworldCharacterPlaybackHistory.h"
#include "InworldCharacterPlaybackTrigger.h"
#include "InworldCharacterAudioComponent.h"

UInworldMetahumanEditorSettings::UInworldMetahumanEditorSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	MetahumanBodySkeleton = FSoftObjectPath{ TEXT("Skeleton'/Game/MetaHumans/Common/Female/Medium/NormalWeight/Body/metahuman_base_skel.metahuman_base_skel'") };
	MetahumanBodyABP = FSoftObjectPath{ TEXT("AnimBlueprint'/InworldMetahuman/Body/ABP_Inworld_Metahuman.ABP_Inworld_Metahuman'") };

	MetahumanFaceSkeleton = FSoftObjectPath{ TEXT("Skeleton'/Game/MetaHumans/Common/Face/Face_Archetype_Skeleton.Face_Archetype_Skeleton'") };
	MetahumanFaceABP = FSoftObjectPath{ TEXT("AnimBlueprint'/InworldMetahuman/Face/ABP_Inworld_Metahuman_Face.ABP_Inworld_Metahuman_Face'") };

	CharacterPlaybacks = { };
	CharacterComponents = { };
}
