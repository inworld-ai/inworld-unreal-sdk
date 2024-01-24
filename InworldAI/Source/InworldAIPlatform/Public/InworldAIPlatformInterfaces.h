/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

namespace Inworld
{
    namespace Platform
    {
        enum class Permission
        {
            GRANTED,
            DENIED,
            UNDETERMINED,
        };

        class IMicrophone
        {
        public:
            using RequestAccessCallback = void (*)(bool);

            virtual ~IMicrophone() = default;
            virtual void RequestAccess(RequestAccessCallback Callback) = 0;
            virtual Permission GetPermission() const = 0;
        };
    }
}
