/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldEditorUIStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Styling/SlateTypes.h"
#include "Styling/CoreStyle.h"
#include "Interfaces/IPluginManager.h"
#include "SlateOptMacros.h"


#define IMAGE_BRUSH(RelativePath, ...) FSlateImageBrush(StyleSet->RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#define BOX_BRUSH( RelativePath, ... ) FSlateBoxBrush(StyleSet->RootToContentDir(RelativePath, TEXT(".png") ), __VA_ARGS__)
#define BORDER_BRUSH( RelativePath, ... ) FSlateBorderBrush(StyleSet->RootToContentDir(RelativePath, TEXT(".png") ), __VA_ARGS__)

TSharedPtr< FSlateStyleSet > FInworldEditorUIStyle::StyleSet = nullptr;
TSharedPtr< class ISlateStyle > FInworldEditorUIStyle::Get() { return StyleSet; }

FName FInworldEditorUIStyle::GetStyleSetName()
{
	static FName InworldStyleName(TEXT("InworldEditorUIStyle"));
	return InworldStyleName;
}

void FInworldEditorUIStyle::Initialize()
{
	// Const icon sizes
	const FVector2D Icon8x8(8.0f, 8.0f);
	const FVector2D Icon16x16(16.0f, 16.0f);
	const FVector2D Icon20x20(20.0f, 20.0f);
	const FVector2D Icon40x40(40.0f, 40.0f);
	const FVector2D Icon64x64(64.0f, 64.0f);

	// Only register once
	if (StyleSet.IsValid())
	{
		return;
	}

	StyleSet = MakeShareable(new FSlateStyleSet(GetStyleSetName()));

	StyleSet->SetContentRoot(IPluginManager::Get().FindPlugin(TEXT("InworldAI"))->GetContentDir());
	StyleSet->SetCoreContentRoot(FPaths::EngineContentDir() / TEXT("Slate"));

	// Inworld Asset Editor
	StyleSet->Set("InworldEditor.Icon", new IMAGE_BRUSH("Icons/InworldIcon_40x", Icon40x40));

	StyleSet->Set("InworldEditor.NarrativeGraph.Body", new BOX_BRUSH("/NarrativeGraph/GraphNodeBody", FMargin(16.f / 64.f, 25.f / 64.f, 16.f / 64.f, 16.f / 64.f)));
	StyleSet->Set("InworldEditor.NarrativeGraph.ColorSpill", new BOX_BRUSH("/NarrativeGraph/GraphNodeColorSpill", FMargin(4.0f / 64.0f, 4.0f / 32.0f)));

	FSlateStyleRegistry::RegisterSlateStyle(*StyleSet.Get());
};

#undef IMAGE_BRUSH

void FInworldEditorUIStyle::Shutdown()
{
	if (StyleSet.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSet.Get());
		ensure(StyleSet.IsUnique());
		StyleSet.Reset();
	}
}
