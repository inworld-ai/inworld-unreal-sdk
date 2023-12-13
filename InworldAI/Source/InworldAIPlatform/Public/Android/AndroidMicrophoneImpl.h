// Copyright 2023 Theai, Inc. (DBA Inworld) All Rights Reserved.

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
