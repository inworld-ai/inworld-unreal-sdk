/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#if PLATFORM_ANDROID

#include "AndroidMicrophoneImpl.h"
#include "AndroidPermissionFunctionLibrary.h"
#include "AndroidPermissionCallbackProxy.h"

void Inworld::Platform::AndroidMicrophoneImpl::RequestAccess(RequestAccessCallback Callback)
{
	UAndroidPermissionCallbackProxy* ProxyObj =
		UAndroidPermissionFunctionLibrary::AcquirePermissions({ "android.permission.RECORD_AUDIO" });
	ProxyObj->OnPermissionsGrantedDelegate.AddLambda([this, Callback](const TArray<FString>& Permissions, const TArray<bool>& GrantResults)
		{
			const bool bGranted = !GrantResults.IsEmpty() ? GrantResults[0] : false;
			CurrentPermission = bGranted ? Permission::GRANTED : Permission::DENIED;
			Callback(bGranted);
		});
}

#endif

