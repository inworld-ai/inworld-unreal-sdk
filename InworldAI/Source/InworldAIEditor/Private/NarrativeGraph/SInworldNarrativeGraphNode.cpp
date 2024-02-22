// Copyright Epic Games, Inc. All Rights Reserved.

#include "SInworldNarrativeGraphNode.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBox.h"
#include "SGraphPin.h"
#include "SLevelOfDetailBranchNode.h"
#include "NarrativeGraph/InworldNarrativeGraphNode.h"

#define LOCTEXT_NAMESPACE "ReferenceViewer"

class SInworldNarrativeGraphPin : public SGraphPin
{
public:
	SLATE_BEGIN_ARGS(SInworldNarrativeGraphPin) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UEdGraphPin* InPin);
protected:
	// Begin SGraphPin interface
	virtual TSharedRef<SWidget>	GetDefaultValueWidget() override;
	// End SGraphPin interface

	const FSlateBrush* GetPinBorder() const;
};

void SInworldNarrativeGraphPin::Construct(const FArguments& InArgs, UEdGraphPin* InPin)
{
	SetCursor(EMouseCursor::Default);

	bShowLabel = true;

	GraphPinObj = InPin;
	check(GraphPinObj != NULL);

	const UEdGraphSchema* Schema = GraphPinObj->GetSchema();
	check(Schema);

	// Set up a hover for pins that is tinted the color of the pin.
	SBorder::Construct(SBorder::FArguments()
		.BorderBackgroundColor(this, &SInworldNarrativeGraphPin::GetPinColor)
		.OnMouseButtonDown(this, &SInworldNarrativeGraphPin::OnPinMouseDown)
		.Cursor(this, &SInworldNarrativeGraphPin::GetPinCursor)
	);
}

TSharedRef<SWidget>	SInworldNarrativeGraphPin::GetDefaultValueWidget()
{
	return SNew(STextBlock);
}

const FSlateBrush* SInworldNarrativeGraphPin::GetPinBorder() const
{
	return (IsHovered())
		? FEditorStyle::GetBrush(TEXT("Graph.StateNode.Pin.BackgroundHovered"))
		: FEditorStyle::GetBrush(TEXT("Graph.StateNode.Pin.Background"));
}

void SInworldNarrativeGraphNode::Construct( const FArguments& InArgs, UInworldNarrativeGraphNode* InNode )
{
	GraphNode = InNode;
	SetCursor(EMouseCursor::CardinalCross);
	UpdateGraphNode();
}

void SInworldNarrativeGraphNode::CreatePinWidgets()
{
	UInworldNarrativeGraphNode* StateNode = CastChecked<UInworldNarrativeGraphNode>(GraphNode);

	for (int32 PinIdx = 0; PinIdx < StateNode->Pins.Num(); PinIdx++)
	{
		UEdGraphPin* MyPin = StateNode->Pins[PinIdx];
		if (!MyPin->bHidden)
		{
			TSharedPtr<SGraphPin> NewPin = SNew(SInworldNarrativeGraphPin, MyPin);

			AddPin(NewPin.ToSharedRef());
		}
	}
}

void SInworldNarrativeGraphNode::AddPin(const TSharedRef<SGraphPin>& PinToAdd)
{
	PinToAdd->SetOwner(SharedThis(this));
	RightNodeBox->AddSlot()
		[
			PinToAdd
		];
	OutputPins.Add(PinToAdd);
}

void SInworldNarrativeGraphNode_Root::Construct(const FArguments& InArgs, UInworldNarrativeGraphNode* InNode)
{
	GraphNode = InNode;
	UpdateGraphNode();
}

const FSlateBrush* SInworldNarrativeGraphNode::GetNameIcon() const
{
	return FEditorStyle::GetBrush(TEXT("Graph.StateNode.Icon"));
}

FSlateColor SInworldNarrativeGraphNode::GetBorderBackgroundColor() const
{
	return FLinearColor{134.f/255.f, 76.f/255.f, 255.f/255.f};
}

void SInworldNarrativeGraphNode_Root::UpdateGraphNode()
{
	InputPins.Empty();
	OutputPins.Empty();

	RightNodeBox.Reset();
	LeftNodeBox.Reset();

	TSharedPtr<STextBlock> TextBox;
	TSharedPtr<SNodeTitle> NodeTitle = SNew(SNodeTitle, GraphNode);

	ContentScale.Bind(this, &SGraphNode::GetContentScale);
	GetOrAddSlot(ENodeZone::Center)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SBorder)
				.BorderImage(FEditorStyle::GetBrush("Graph.Node.TintedBody"))
				.Padding(0)
				.BorderBackgroundColor(this, &SInworldNarrativeGraphNode::GetBorderBackgroundColor)
				[
					SNew(SOverlay)

						// PIN AREA
						+ SOverlay::Slot()
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
						.Padding(10.0f)
						[
							SAssignNew(RightNodeBox, SVerticalBox)
						]

						// STATE NAME AREA
						+ SOverlay::Slot()
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						.Padding(10.0f)
						[
							SNew(SBorder)
								.BorderImage(FEditorStyle::GetBrush("Graph.Node.Body"))
								.HAlign(HAlign_Center)
								.VAlign(VAlign_Center)
								.Visibility(EVisibility::SelfHitTestInvisible)
								[
									SNew(SHorizontalBox)
										+ SHorizontalBox::Slot()
										.Padding(FMargin(4.0f, 0.0f, 4.0f, 0.0f))
										[
											SNew(SVerticalBox)
												+ SVerticalBox::Slot()
												.AutoHeight()
												[
													SAssignNew(TextBox, STextBlock)
														.Text(NodeTitle.Get(), &SNodeTitle::GetHeadTitle)
												]
												+ SVerticalBox::Slot()
												.AutoHeight()
												[
													NodeTitle.ToSharedRef()
												]
										]
								]
						]
				]
		];

	CreatePinWidgets();
}

FText SInworldNarrativeGraphNode_Root::GetPreviewCornerText() const
{
	return NSLOCTEXT("SGraphNodeAnimStateEntry", "CornerTextDescription", "Entry point for state machine");
}

void SInworldNarrativeGraphNode_Scene::Construct(const FArguments& InArgs, UInworldNarrativeGraphNode* InNode)
{
	GraphNode = InNode;
	UpdateGraphNode();
}

void SInworldNarrativeGraphNode_Scene::UpdateGraphNode()
{
	InputPins.Empty();
	OutputPins.Empty();

	RightNodeBox.Reset();
	LeftNodeBox.Reset();

	TSharedPtr<SVerticalBox> MainVerticalBox;
	TSharedPtr<SNodeTitle> NodeTitle = SNew(SNodeTitle, GraphNode);

	TSharedRef<SOverlay> TitleAreaWidget =
		SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SImage)
				.Image(FEditorStyle::GetBrush("Graph.Node.TitleGloss"))
				.ColorAndOpacity(this, &SGraphNode::GetNodeTitleIconColor)
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Center)
		[
			SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Fill)
				[
					SNew(SBorder)
						.BorderImage(FEditorStyle::GetBrush("Graph.Node.ColorSpill"))
						// The extra margin on the right
						// is for making the color spill stretch well past the node title
						.Padding(FMargin(10, 5, 30, 3))
						.BorderBackgroundColor(this, &SGraphNode::GetNodeTitleColor)
						[
							SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								[
									SNew(SVerticalBox)
										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											CreateTitleWidget(NodeTitle)
										]
										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											NodeTitle.ToSharedRef()
										]
								]
						]
				]
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Center)
				.Padding(0, 0, 5, 0)
				.AutoWidth()
				[
					CreateTitleRightWidget()
				]
		]
		+ SOverlay::Slot()
		.VAlign(VAlign_Top)
		[
			SNew(SBorder)
				.Visibility(EVisibility::HitTestInvisible)
				.BorderImage(FEditorStyle::GetBrush("Graph.Node.TitleHighlight"))
				.BorderBackgroundColor(this, &SGraphNode::GetNodeTitleIconColor)
				[
					SNew(SSpacer)
						.Size(FVector2D(20, 20))
				]
		];

	SetDefaultTitleAreaWidget(TitleAreaWidget);

	UInworldNarrativeGraphNode* StateNode = CastChecked<UInworldNarrativeGraphNode>(GraphNode);

	TSharedRef<SOverlay> ContentAreaWidget =
		SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SBox)
				.WidthOverride(200.f)
				[
				SNew(SVerticalBox)
				+SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					.Padding(25.0f)
					[
						SNew(STextBlock)
							.Justification(ETextJustify::Left)
							.Text(FText::AsCultureInvariant(StateNode->Scene))
							.AutoWrapText(true)
					]
				]
		];

	ContentScale.Bind(this, &SGraphNode::GetContentScale);
	GetOrAddSlot(ENodeZone::Center)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SBorder)
				.BorderImage(FEditorStyle::GetBrush("Graph.Node.Body"))
				.Padding(0)
				.BorderBackgroundColor(this, &SInworldNarrativeGraphNode::GetBorderBackgroundColor)
				[
					SNew(SOverlay)

						// PIN AREA
						+ SOverlay::Slot()
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
						.Padding(10.0f)
						[
							SAssignNew(RightNodeBox, SVerticalBox)
						]

						+ SOverlay::Slot()
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						.Padding(10.0f)
						.Padding(Settings->GetNonPinNodeBodyPadding())
						[
							SNew(SImage)
								.Image(GetNodeBodyBrush())
								.ColorAndOpacity(this, &SGraphNode::GetNodeBodyColor)
						]
						+ SOverlay::Slot()
						[
							SNew(SVerticalBox)
								+ SVerticalBox::Slot()
								.AutoHeight()
								.HAlign(HAlign_Fill)
								.VAlign(VAlign_Top)
								.Padding(Settings->GetNonPinNodeBodyPadding())
								[
									TitleAreaWidget
								]
								+ SVerticalBox::Slot()
								.AutoHeight()
								.HAlign(HAlign_Fill)
								.VAlign(VAlign_Fill)
								[
									ContentAreaWidget
								]
						]
				]
		];
	
	CreatePinWidgets();
}

#undef LOCTEXT_NAMESPACE
