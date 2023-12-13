// Copyright 2023 Theai, Inc. (DBA Inworld) All Rights Reserved.

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
