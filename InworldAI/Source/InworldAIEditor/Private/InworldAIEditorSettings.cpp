// Copyright 2023 Theai, Inc. (DBA Inworld) All Rights Reserved.


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
