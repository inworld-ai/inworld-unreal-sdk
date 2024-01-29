/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

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
