// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "InworldNarrativeGraphEditorWidget.generated.h"

/**
 * 
 */
UCLASS()
class UInworldNarrativeGraphEditorWidget : public UEditorUtilityWidget
{
	GENERATED_BODY()

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
};
