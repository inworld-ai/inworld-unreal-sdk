// Copyright 2023 Theai, Inc. (DBA Inworld) All Rights Reserved.

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
