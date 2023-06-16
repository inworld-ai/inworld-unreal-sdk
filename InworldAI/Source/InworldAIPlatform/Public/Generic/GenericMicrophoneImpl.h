/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#ifdef INWORLD_PLATFORM_GENERIC

#include "InworldAIPlatformInterfaces.h"

namespace Inworld
{
    namespace Platform
    {
        class GenericMicrophoneImpl : public IMicrophone
        {
        public:
            GenericMicrophoneImpl();
            virtual ~GenericMicrophoneImpl();

            virtual void RequestAccess(RequestAccessCallback Callback) override;
            virtual Permission GetPermission() const override;
        };
    }
}

#endif
