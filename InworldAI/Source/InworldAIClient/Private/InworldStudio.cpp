/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldStudio.h"
#include "InworldAIClientModule.h"

#include "Async/Async.h"
#include "Async/TaskGraphInterfaces.h"

THIRD_PARTY_INCLUDES_START
// UNREAL ENGINE 4
#pragma warning(push)
#pragma warning(disable:4583)
#pragma warning(disable:4582)
#include "GrpcHelpers.h"
#pragma warning(pop)
// UNREAL ENGINE 4
#include "StudioClient.h"
THIRD_PARTY_INCLUDES_END

#include <string>

FInworldStudioUserData ConvertStudioUserData(const Inworld::StudioUserData& Data)
{
	FInworldStudioUserData D;
	for (const auto& W : Data.Workspaces)
	{
		auto& WNew = D.Workspaces.Emplace_GetRef();
		WNew.Name = UTF8_TO_TCHAR(W.Name.c_str());
		WNew.ShortName = UTF8_TO_TCHAR(W.ShortName.c_str());

		WNew.ApiKeys.Reserve(W.ApiKeys.size());
		for (const auto& A : W.ApiKeys)
		{
			auto& ANew = WNew.ApiKeys.Emplace_GetRef();
			ANew.Name = UTF8_TO_TCHAR(A.Name.c_str());
			ANew.Key = UTF8_TO_TCHAR(A.Key.c_str());
			ANew.Secret = UTF8_TO_TCHAR(A.Secret.c_str());
			ANew.IsActive = A.IsActive;
		}

		WNew.Characters.Reserve(W.Characters.size());
		for (const auto& C : W.Characters)
		{
			auto& CNew = WNew.Characters.Emplace_GetRef();
			CNew.Name = UTF8_TO_TCHAR(C.Name.c_str());
			CNew.ShortName = UTF8_TO_TCHAR(C.ShortName.c_str());
			CNew.RpmImageUri = UTF8_TO_TCHAR(C.RpmImageUri.c_str());
			CNew.RpmModelData = UTF8_TO_TCHAR(C.RpmModelData.c_str());
			CNew.RpmModelUri = UTF8_TO_TCHAR(C.RpmModelUri.c_str());
			CNew.RpmPortraitUri = UTF8_TO_TCHAR(C.RpmPortraitUri.c_str());
			CNew.RpmPostureUri = UTF8_TO_TCHAR(C.RpmPostureUri.c_str());
			CNew.bMale = C.bMale;
		}

		WNew.Scenes.Reserve(W.Scenes.size());
		for (const auto& S : W.Scenes)
		{
			auto& SNew = WNew.Scenes.Emplace_GetRef();
			SNew.Name = UTF8_TO_TCHAR(S.Name.c_str());
			SNew.ShortName = UTF8_TO_TCHAR(S.ShortName.c_str());

			SNew.Characters.Reserve(S.Characters.size());
			for (const auto& C : S.Characters)
			{
				SNew.Characters.Add(UTF8_TO_TCHAR(C.c_str()));
			}
		}
	}
	return D;
}

FInworldStudio::FInworldStudio()
{
	Inworld::CreateStudioClient();
}

FInworldStudio::~FInworldStudio()
{
	Inworld::DestroyStudioClient();
}

void FInworldStudio::CancelRequests()
{
	Inworld::GetStudioClient()->CancelRequests();
}

bool FInworldStudio::IsRequestInProgress() const
{
	return Inworld::GetStudioClient()->IsRequestInProgress();
}

void FInworldStudio::RequestStudioUserData(const FString& Token, const FString& ServerUrl, TFunction<void(bool bSuccess)> InCallback)
{
	Inworld::GetStudioClient()->RequestStudioUserDataAsync(TCHAR_TO_UTF8(*Token), TCHAR_TO_UTF8(*ServerUrl), [InCallback](bool bSuccess)
		{
			AsyncTask(ENamedThreads::GameThread, [InCallback, bSuccess]()
				{
					InCallback(bSuccess);
				});
		});	
}

FString FInworldStudio::GetError() const
{
	return UTF8_TO_TCHAR(Inworld::GetStudioClient()->GetError().c_str());
}

FInworldStudioUserData FInworldStudio::GetStudioUserData() const
{
	if (Data.Workspaces.Num() == 0)
	{
		Data = ConvertStudioUserData(Inworld::GetStudioClient()->GetStudioUserData());
	}
	return Data;
}
