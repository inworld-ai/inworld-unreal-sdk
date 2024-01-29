/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#if PLATFORM_IOS || PLATFORM_MAC

#import <AVFoundation/AVFoundation.h>

@interface AppleAudioPermission : NSObject

typedef void (*Callback)(bool);

-(void)requestAccess: (Callback) callback;
-(int)getPermission;

@end

@implementation AppleAudioPermission

-(instancetype)init
{
    self = [super init];
    return self;
}

-(int) getPermission
{
    switch([AVCaptureDevice authorizationStatusForMediaType : AVMediaTypeAudio])
    {
        case AVAuthorizationStatusAuthorized:
            return 0;
            break;
        case AVAuthorizationStatusNotDetermined:
            return 1;
            break;
        default:
            return 2;
            break;
    }
}

-(void) requestAccess:(Callback)callback
{
    switch([AVCaptureDevice authorizationStatusForMediaType : AVMediaTypeAudio])
    {
        case AVAuthorizationStatusAuthorized:
            // Audio is granted access
            callback(true);
            break;
        case AVAuthorizationStatusNotDetermined:
            [AVCaptureDevice requestAccessForMediaType : AVMediaTypeAudio completionHandler : ^ (BOOL Granted) {
                callback(Granted);
            }];
            break;
        case AVAuthorizationStatusDenied:
        case AVAuthorizationStatusRestricted:
        default:
            callback(false);
            break;
    }
}

@end

#endif
