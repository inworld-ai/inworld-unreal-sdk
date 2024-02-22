// Copyright Epic Games, Inc. All Rights Reserved.

#include "NarrativeGraph/InworldNarrativeGraphNode.h"

#define LOCTEXT_NAMESPACE "InworldNarrativeGraphNode"

FLinearColor UInworldNarrativeGraphNode::GetNodeTitleColor() const
{
	return FLinearColor(134.f/255.f, 76.f/255.f, 255.f/255.f);
}

void UInworldNarrativeGraphNode::AllocateDefaultPins()
{
	TArray<FString> InputNames;
	Inputs.GetKeys(InputNames);
	for (const auto& InputName : InputNames)
	{
		Inputs.Add(InputName, CreatePin(EGPD_Input, TEXT("Transition"), FName(InputName)));
	}
	TArray<FString> OutputNames;
	Outputs.GetKeys(OutputNames);
	for (const auto& OutputName : OutputNames)
	{
		Outputs.Add(OutputName, CreatePin(EGPD_Output, TEXT("Transition"), FName(OutputName)));
	}
}

FText UInworldNarrativeGraphNode_Root::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("InworldNarrativeGraphNode_Root_Title", "Start");
}

FText UInworldNarrativeGraphNode_Root::GetTooltipText() const
{
	return LOCTEXT("InworldNarrativeGraphNode_Root_Tooltip", "");
}

FText UInworldNarrativeGraphNode_Scene::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::AsCultureInvariant(Name);
}

FText UInworldNarrativeGraphNode_Scene::GetTooltipText() const
{
	return LOCTEXT("InworldNarrativeGraphNode_Scene_Tooltip", "");
}

#undef LOCTEXT_NAMESPACE
