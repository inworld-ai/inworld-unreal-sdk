// Copyright 2023 Theai, Inc. (DBA Inworld) All Rights Reserved.

#ifdef INWORLD_PLATFORM_GENERIC

#include "GenericMicrophoneImpl.h"

namespace Inworld
{
    namespace Platform
    {
        GenericMicrophoneImpl::GenericMicrophoneImpl()
        {
        }

        GenericMicrophoneImpl::~GenericMicrophoneImpl()
        {
        }

        Permission GenericMicrophoneImpl::GetPermission() const
        {
            // Assume microphone access is always granted for generic
            return Permission::GRANTED;
        }

        void GenericMicrophoneImpl::RequestAccess(RequestAccessCallback Callback)
        {
            // Assume microphone is already accessed for generic
            Callback(true);
        }
    }
}

#endif
