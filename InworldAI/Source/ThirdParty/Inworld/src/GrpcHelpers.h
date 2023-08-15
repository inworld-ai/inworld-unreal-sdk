/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "ai/inworld/studio/v1alpha/characters.pb.h"

#include <string>

namespace InworldV1alpha = ai::inworld::studio::v1alpha;

namespace Inworld
{
	namespace GrpcHelper
	{
		class CharacterInfo
		{
		public:
			CharacterInfo() = default;
			CharacterInfo(const InworldV1alpha::Character& GrpcCharacter)
				: _Name(GrpcCharacter.name())
				, _RpmModelUri(GrpcCharacter.default_character_assets().rpm_model_uri())
				, _RpmImageUri(GrpcCharacter.default_character_assets().rpm_image_uri())
				, _RpmPortraitUri(GrpcCharacter.default_character_assets().rpm_image_uri_portrait())
				, _RpmPostureUri(GrpcCharacter.default_character_assets().rpm_image_uri_posture())
				, _bMale(GrpcCharacter.default_character_description().pronoun() == InworldV1alpha::Character_CharacterDescription_Pronoun_PRONOUN_MALE)
			{}
			~CharacterInfo() = default;

			std::string _Name;
			std::string _RpmModelUri;
			std::string _RpmImageUri;
			std::string _RpmPortraitUri;
			std::string _RpmPostureUri;
			bool _bMale;
		};

		CharacterInfo CreateCharacterInfo(const InworldV1alpha::Character& GrpcCharacter);
		
	}
}