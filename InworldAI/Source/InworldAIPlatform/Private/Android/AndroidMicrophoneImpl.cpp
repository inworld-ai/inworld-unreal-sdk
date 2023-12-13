// Copyright 2023 Theai, Inc. (DBA Inworld) All Rights Reserved.

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

