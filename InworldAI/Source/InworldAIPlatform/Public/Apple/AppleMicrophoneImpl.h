/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#if PLATFORM_IOS || PLATFORM_MAC

#include "InworldAIPlatformInterfaces.h"
#import "AppleAudioPermission.h"

namespace Inworld
{
    namespace Platform
    {
        class AppleMicrophoneImpl : public IMicrophone
        {
        public:
            AppleMicrophoneImpl()
            {
                obj = [[AppleAudioPermission alloc]init];
            }
            
            virtual ~AppleMicrophoneImpl()
            {
                obj = nil;
            }

            virtual void RequestAccess(RequestAccessCallback Callback) override
            {
                [obj requestAccess : Callback];
            }
            
            virtual Permission GetPermission() const override
            {
                switch ([obj getPermission])
                {
                case 0:
                    return Permission::GRANTED;
                case 1:
                    return Permission::UNDETERMINED;
                default:
                    return Permission::DENIED;
                }
            }

        private:
            AppleAudioPermission* obj;
        };
    }
}

#endif
