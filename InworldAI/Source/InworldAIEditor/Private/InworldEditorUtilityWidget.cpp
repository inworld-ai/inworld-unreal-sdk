/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "InworldEditorUtilityWidget.h"
#include "InworldAIEditorModule.h"

bool UInworldEditorUtilityWidget::Initialize()
{
	if (Super::Initialize())
	{
		FInworldAIEditorModule* Module = static_cast<FInworldAIEditorModule*>(FModuleManager::Get().GetModule("InworldAIEditor"));
		if (ensure(Module))
		{
			OnInitializedReal(Module->GetStudioWidgetState());
		}
		
		return true;
	}

	return false;
}

void UInworldEditorUtilityWidget::UpdateState(const FInworldEditorUtilityWidgetState& State)
{
	FInworldAIEditorModule* Module = static_cast<FInworldAIEditorModule*>(FModuleManager::Get().GetModule("InworldAIEditor"));
	if (ensure(Module))
	{
		return Module->SetStudioWidgetState(State);
	}
}
