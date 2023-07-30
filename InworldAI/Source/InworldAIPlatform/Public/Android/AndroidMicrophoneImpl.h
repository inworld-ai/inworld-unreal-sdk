/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#if PLATFORM_ANDROID

#include "InworldAIPlatformInterfaces.h"

namespace Inworld
{
    namespace Platform
    {
        class AndroidMicrophoneImpl : public IMicrophone
        {
        public:
            AndroidMicrophoneImpl() = default;
            virtual ~AndroidMicrophoneImpl() = default;

            virtual void RequestAccess(RequestAccessCallback Callback) override;
            
            virtual Permission GetPermission() const override
            {
                return CurrentPermission;
            }

        private:
            Permission CurrentPermission = Permission::UNDETERMINED;
        };
    }
}

#endif
