/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */


#include "InworldAIEditorSettings.h"
#include "UObject/ConstructorHelpers.h"
#include "Sound/SoundSubmix.h"

#include "InworldPlayerComponent.h"
#include "InworldPlayerAudioCaptureComponent.h"
#include "InworldPlayerTargetingComponent.h"

#include "InworldCharacterComponent.h"
#include "InworldCharacterPlaybackTrigger.h"
#include "InworldCharacterAudioComponent.h"

UInworldAIEditorSettings::UInworldAIEditorSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InworldPlayerComponent = UInworldPlayerComponent::StaticClass();

	OtherPlayerComponents = { UInworldPlayerAudioCaptureComponent::StaticClass(), UInworldPlayerTargetingComponent::StaticClass() };

	InworldCharacterComponent = UInworldCharacterComponent::StaticClass();
	CharacterPlaybacks = { UInworldCharacterPlaybackTrigger::StaticClass() };
	OtherCharacterComponents = { UInworldCharacterAudioComponent::StaticClass() };

	InworldStudioWidget = "/InworldAI/StudioWidget/InworldStudioWidget.InworldStudioWidget";
}
