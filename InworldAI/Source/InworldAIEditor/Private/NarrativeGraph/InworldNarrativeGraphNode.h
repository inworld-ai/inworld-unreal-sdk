// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "EdGraph/EdGraphNode.h"
#include "InworldStudioTypes.h"
#include "InworldNarrativeGraphNode.generated.h"

class UEdGraphPin;
class USoundNode;

UCLASS(MinimalAPI)
class UInworldNarrativeGraphNode : public UEdGraphNode
{
	GENERATED_BODY()

public:
	FString Name;
	FString Scene;
	TMap<FString, UEdGraphPin*> Inputs;
	TMap<FString, UEdGraphPin*> Outputs;

	//~ Begin UEdGraphNode Interface
	virtual void AllocateDefaultPins() override;
	virtual FLinearColor GetNodeTitleColor() const override;
	//~ End UEdGraphNode Interface
};

UCLASS(MinimalAPI)
class UInworldNarrativeGraphNode_Root : public UInworldNarrativeGraphNode
{
	GENERATED_BODY()

public:
	//~ Begin UEdGraphNode Interface
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	//~ End UEdGraphNode Interface
};

UCLASS(MinimalAPI)
class UInworldNarrativeGraphNode_Scene : public UInworldNarrativeGraphNode
{
	GENERATED_BODY()

public:
	//~ Begin UEdGraphNode Interface
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	//~ End UEdGraphNode Interface
};
