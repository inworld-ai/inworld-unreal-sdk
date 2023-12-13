// Copyright 2023 Theai, Inc. (DBA Inworld) All Rights Reserved.

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
