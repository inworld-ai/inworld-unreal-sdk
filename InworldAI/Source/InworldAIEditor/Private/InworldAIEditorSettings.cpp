/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */


#include "InworldAIEditorSettings.h"
#include "UObject/ConstructorHelpers.h"
#include "Sound/SoundSubmix.h"
#include "InworldPlayerAudioCaptureComponent.h"
#include "InworldPlayerTargetingComponent.h"
#include "InworldCharacterPlaybackAudio.h"
#include "Components/AudioComponent.h"

UInworldAIEditorSettings::UInworldAIEditorSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InworldPlayerComponent = UInworldPlayerComponent::StaticClass();

	OtherPlayerComponents = { UInworldPlayerAudioCaptureComponent::StaticClass(), UInworldPlayerTargetingComponent::StaticClass() };

	InworldCharacterComponent = UInworldCharacterComponent::StaticClass();
	CharacterPlaybacks = { UInworldCharacterPlaybackAudio::StaticClass() };
	OtherCharacterComponents = { UAudioComponent::StaticClass() };
}
