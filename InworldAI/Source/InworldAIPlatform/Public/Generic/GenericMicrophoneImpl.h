// Copyright 2023 Theai, Inc. (DBA Inworld) All Rights Reserved.

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
