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
#import <AVFoundation/AVAudioSession.h>

namespace Inworld
{
    namespace Platform
    {
        class AppleMicrophoneImpl : public IMicrophone
        {
        public:
            AppleMicrophoneImpl()
            {
                permission = [[AppleAudioPermission alloc]init];
            }
            
            virtual ~AppleMicrophoneImpl()
            {
                permission = nil;
            }

            virtual bool Initialize() override
            {
#if PLATFORM_IOS
                NSError* setCategoryError = nil;
                [[AVAudioSession sharedInstance]setCategory:AVAudioSessionCategoryPlayAndRecord withOptions : (AVAudioSessionCategoryOptionDefaultToSpeaker | AVAudioSessionCategoryOptionAllowBluetooth) error : &setCategoryError];
                  if (setCategoryError != nil)
                  {
                    return false;
                  }
                
                NSError* setActiveError = nil;
                [[AVAudioSession sharedInstance]setActive:YES error : &setActiveError];
                return nil == setActiveError;
#elif PLATFORM_MAC
                return true;
#endif
            }

            virtual void RequestAccess(RequestAccessCallback Callback) override
            {
                [permission requestAccess : Callback];
            }
            
            virtual Permission GetPermission() const override
            {
                switch ([permission getPermission])
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
            AppleAudioPermission* permission;
        };
    }
}

#endif
