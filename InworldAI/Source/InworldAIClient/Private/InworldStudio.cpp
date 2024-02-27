/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldStudio.h"
#include "InworldAIClientModule.h"
#include "InworldAsyncRoutine.h"

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

namespace Inworld
{
	class FStudio : public StudioClientBase
	{

	public:
		TWeakPtr<FStudio> SelfWeakPtr;

		FInworldStudioUserData GetData() const
		{
			if (Data.Workspaces.IsEmpty())
			{
				Data = ConvertStudioUserData(GetStudioUserData());
			}
			return Data;
		}

	protected:
		virtual void AddTaskToMainThread(std::function<void()> Task) override;

		mutable FInworldStudioUserData Data;
	};
}

FInworldStudio::FInworldStudio()
{
	InworldStudio = MakeShared<Inworld::FStudio>();
	InworldStudio->SelfWeakPtr = InworldStudio;
}

FInworldStudio::~FInworldStudio()
{
	InworldStudio.Reset();
}

void FInworldStudio::CancelRequests()
{
	InworldStudio->CancelRequests();
}

bool FInworldStudio::IsRequestInProgress() const
{
	return InworldStudio->IsRequestInProgress();
}

void FInworldStudio::RequestStudioUserData(const FString& Token, const FString& ServerUrl, TFunction<void(bool bSuccess)> InCallback)
{
	InworldStudio->RequestStudioUserData(TCHAR_TO_UTF8(*Token), TCHAR_TO_UTF8(*ServerUrl), InCallback);
}

FString FInworldStudio::GetError() const
{
	return UTF8_TO_TCHAR(InworldStudio->GetError().c_str());
}

FInworldStudioUserData FInworldStudio::GetStudioUserData() const
{

	return InworldStudio->GetData();
}

void Inworld::FStudio::AddTaskToMainThread(std::function<void()> Task)
{
	AsyncTask(ENamedThreads::GameThread, [Task, SelfPtr = SelfWeakPtr]()
		{
			if (SelfPtr.IsValid())
			{
				Task();
			}
		}
	);
}
