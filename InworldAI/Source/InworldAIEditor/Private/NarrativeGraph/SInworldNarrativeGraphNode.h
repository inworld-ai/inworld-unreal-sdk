// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "SGraphNode.h"

class FAssetThumbnail;
class UInworldNarrativeGraphNode;

class SInworldNarrativeGraphNode : public SGraphNode
{
public:
	SLATE_BEGIN_ARGS(SInworldNarrativeGraphNode){}

	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct( const FArguments& InArgs, UInworldNarrativeGraphNode* InNode );

	// SNodePanel::SNode interface
	virtual void GetNodeInfoPopups(FNodeInfoContext* Context, TArray<FGraphInformationPopupInfo>& Popups) const override {}
	// End SNodePanel::SNode interface

	// SGraphNode implementation
	virtual bool IsNodeEditable() const override { return false; }
	virtual void CreatePinWidgets();
	virtual void AddPin(const TSharedRef<SGraphPin>& PinToAdd) override;
	// End SGraphNode implementation

	virtual const FSlateBrush* GetNameIcon() const;
	virtual FSlateColor GetBorderBackgroundColor() const;
};

class SInworldNarrativeGraphNode_Root : public SInworldNarrativeGraphNode
{
public:
	SLATE_BEGIN_ARGS(SInworldNarrativeGraphNode_Root) {}

	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs, UInworldNarrativeGraphNode* InNode);

	// SGraphNode implementation
	virtual void UpdateGraphNode() override;
	// End SGraphNode implementation

protected:
	FText GetPreviewCornerText() const;
};

class SInworldNarrativeGraphNode_Scene : public SInworldNarrativeGraphNode
{
public:
	SLATE_BEGIN_ARGS(SInworldNarrativeGraphNode_Scene) {}

	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs, UInworldNarrativeGraphNode* InNode);

	// SGraphNode implementation
	virtual void UpdateGraphNode() override;
	// End SGraphNode implementation
};
