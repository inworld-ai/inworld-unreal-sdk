/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"

UENUM(BlueprintType)
enum class EInworldConnectionState : uint8
{
	Idle,
	Connecting,
	Connected,
	Failed,
	Paused,
	Disconnected,
	Reconnecting
};

UENUM(BlueprintType)
enum class EInworldCharacterGesturePlayback : uint8
{
	UNSPECIFIED = 0,
	INTERACTION = 1,
	INTERACTION_END = 2,
	UTTERANCE = 3,
};

UENUM(BlueprintType)
enum class EInworldCharacterEmotionalBehavior : uint8
{
	NEUTRAL = 0,
	DISGUST = 1,
	CONTEMPT = 2,
	BELLIGERENCE = 3,
	DOMINEERING = 4,
	CRITICISM = 5,
	ANGER = 6,
	TENSION = 7,
	TENSE_HUMOR = 8,
	DEFENSIVENESS = 9,
	WHINING = 10,
	SADNESS = 11,
	STONEWALLING = 12,
	INTEREST = 13,
	VALIDATION = 14,
	AFFECTION = 15,
	HUMOR = 16,
	SURPRISE = 17,
	JOY = 18,
};

UENUM(BlueprintType)
enum class EInworldCharacterEmotionStrength : uint8
{
	UNSPECIFIED = 0,
	WEAK = 1,
	STRONG = 2,
	NORMAL = 3,
};

UENUM(BlueprintType)
enum class EInworldActorType : uint8
{
	UNKNOWN = 0,
	PLAYER = 1,
	AGENT = 2,
};

UENUM(BlueprintType)
enum class EInworldControlEventAction : uint8
{
	UNKNOWN = 0,
	AUDIO_SESSION_START = 1,
	AUDIO_SESSION_END = 2,
	INTERACTION_END = 3,
	TTS_PLAYBACK_START = 4,
	TTS_PLAYBACK_END = 5,
};

