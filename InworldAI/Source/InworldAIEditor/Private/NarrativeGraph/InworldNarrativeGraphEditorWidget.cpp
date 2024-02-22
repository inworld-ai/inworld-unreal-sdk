// Fill out your copyright notice in the Description page of Project Settings.


#include "NarrativeGraph/InworldNarrativeGraphEditorWidget.h"
#include "NarrativeGraph/InworldNarrativeGraphSchema.h"
#include "NarrativeGraph/InworldNarrativeGraph.h"

#include "Kismet2/BlueprintEditorUtils.h"


#define LOCTEXT_NAMESPACE "UInworldNarrativeGraphEditor"

TSharedRef<SWidget> UInworldNarrativeGraphEditorWidget::RebuildWidget()
{
	FGraphAppearanceInfo AppearanceInfo;
	AppearanceInfo.CornerText = LOCTEXT("AppearanceCornerText_InworldNarrativeGraph", "INWORLD NARRATIVE GRAPH");

	const UInworldNarrativeGraphSchema* InworldNarrativeGraphSchema = GetDefault<UInworldNarrativeGraphSchema>();
	UInworldNarrativeGraph* NarrativeGraph = Cast<UInworldNarrativeGraph>(FBlueprintEditorUtils::CreateNewGraph(this, NAME_None, UInworldNarrativeGraph::StaticClass(), UInworldNarrativeGraphSchema::StaticClass()));
	InworldNarrativeGraphSchema->CreateDefaultNodesForGraph(*NarrativeGraph);

	return SNew(SGraphEditor)
		.IsEditable(true)
		.Appearance(AppearanceInfo)
		.GraphToEdit(NarrativeGraph)
		.AutoExpandActionMenu(false)
		.ShowGraphStateOverlay(false);
}

#undef LOCTEXT_NAMESPACE