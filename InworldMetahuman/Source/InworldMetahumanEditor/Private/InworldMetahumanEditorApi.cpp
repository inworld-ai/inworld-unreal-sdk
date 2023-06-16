/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "InworldMetahumanEditorApi.h"
#include "InworldEditorApi.h"
#include "InworldUtils.h"
#include "Modules/ModuleManager.h"
#include "InworldMetahumanEditorSettings.h"
#include "InworldAIEditorModule.h"
#include "NDK/Utils/Log.h"

void UInworldMetahumanEditorApi::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	FInworldAIEditorModule& Module = FModuleManager::Get().LoadModuleChecked<FInworldAIEditorModule>("InworldAIEditor");
	Module.BindMenuAssetAction(
		FName("Inworld Metahuman"),
		FName("Character"),
		FText::FromString("Setup as Inworld Metahuman"),
		FText::FromString("Setup this Actor as Inworld Metahuman"),
		FAssetAction::CreateUObject(this, &UInworldMetahumanEditorApi::SetupAssetAsInworldMetahuman),
		FAssetActionPermission::CreateUObject(this, &UInworldMetahumanEditorApi::CanSetupAssetAsInworldMetahuman, false)
	);
}

void UInworldMetahumanEditorApi::Deinitialize()
{
	Super::Deinitialize();
	
	FInworldAIEditorModule& Module = FModuleManager::Get().LoadModuleChecked<FInworldAIEditorModule>("InworldAIEditor");
	Module.UnbindMenuAssetAction(FName("Inworld Metahuman"));
}

bool UInworldMetahumanEditorApi::DoesSupportWorldType(EWorldType::Type WorldType) const
{
    return WorldType == EWorldType::Editor;
}

bool UInworldMetahumanEditorApi::CanSetupAssetAsInworldMetahuman(const FAssetData& AssetData, bool bLogErrors)
{
	auto* Object = AssetData.GetAsset();
	if (!Object)
	{
		if (bLogErrors) Inworld::LogError("UInworldMetahumanEditorApi::CanSetupAssetAsInworldMetahuman couldn't find Object");
		return false;
	}

	auto* Blueprint = Cast<UBlueprint>(Object);
	if (!Blueprint || !Blueprint->SimpleConstructionScript)
	{
		if (bLogErrors) Inworld::LogError("UInworldMetahumanEditorApi::CanSetupAssetAsInworldMetahuman asset should be Blueprint with SimpleConstructionScript");
		return false;
	}

	auto* InworldEditorApi = GetWorld()->GetSubsystem<UInworldEditorApiSubsystem>();

	const UInworldMetahumanEditorSettings* InworldMetahumanEditorSettings = GetDefault<UInworldMetahumanEditorSettings>();

	auto* FaceComponent = Cast<USkeletalMeshComponent>(InworldEditorApi->GetNodeFromBlueprint(Blueprint, "Face"));
	if (!FaceComponent)
	{
		if (bLogErrors) Inworld::LogError("UInworldMetahumanEditorApi::CanSetupAssetAsInworldMetahuman asset should contain a Face Skeletal Mesh");
		return false;
	}

	TSoftObjectPtr<USkeleton> FaceAsset(InworldMetahumanEditorSettings->MetahumanFace);
	FaceAsset.LoadSynchronous();
	if (!FaceAsset.IsValid())
	{
		if (bLogErrors) Inworld::LogError("UInworldMetahumanEditorApi::CanSetupAssetAsInworldMetahuman needs a valid Face Asset");
		return false;
	}

	TSoftObjectPtr<UAnimBlueprint> FaceAnimAsset(InworldMetahumanEditorSettings->MetahumanFaceAnimBP);
	FaceAnimAsset.LoadSynchronous();
	if (!FaceAnimAsset.IsValid())
	{
		if (bLogErrors) Inworld::LogError("UInworldMetahumanEditorApi::CanSetupAssetAsInworldMetahuman needs a valid Face Anim Asset");
		return false;
	}

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION > 0
	USkeletalMesh* FaceMesh = FaceComponent->GetSkeletalMeshAsset();
#else
	USkeletalMesh* FaceMesh = FaceComponent->SkeletalMesh;
#endif
	if (!FaceMesh || FaceMesh->GetSkeleton() != FaceAsset.Get())
	{
		if (bLogErrors) Inworld::LogError("UInworldMetahumanEditorApi::CanSetupAssetAsInworldMetahuman face mesh needs to be Metahuman");
		return false;
	}

	auto* BodyComponent = Cast<USkeletalMeshComponent>(InworldEditorApi->GetNodeFromBlueprint(Blueprint, "Body"));
	if (!BodyComponent)
	{
		if (bLogErrors) Inworld::LogError("UInworldMetahumanEditorApi::CanSetupAssetAsInworldMetahuman asset should contain a Body Skeletal Mesh");
		return false;
	}

	TSoftObjectPtr<USkeleton> BodyAsset(InworldMetahumanEditorSettings->MetahumanBody);
	BodyAsset.LoadSynchronous();
	if (!BodyAsset.IsValid())
	{
		if (bLogErrors) Inworld::LogError("UInworldMetahumanEditorApi::CanSetupAssetAsInworldMetahuman needs a valid Body Asset");
		return false;
	}

	TSoftObjectPtr<UAnimBlueprint> BodyAnimAsset(InworldMetahumanEditorSettings->MetahumanAnimBP);
	BodyAnimAsset.LoadSynchronous();
	if (!BodyAnimAsset.IsValid())
	{
		if (bLogErrors) Inworld::LogError("UInworldMetahumanEditorApi::CanSetupAssetAsInworldMetahuman needs a valid Body Anim Asset");
		return false;
	}

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION > 0
	USkeletalMesh* BodyMesh = BodyComponent->GetSkeletalMeshAsset();
#else
	USkeletalMesh* BodyMesh = BodyComponent->SkeletalMesh;
#endif
	if (!BodyMesh || BodyMesh->GetSkeleton() != BodyAsset.Get())
	{
		if (bLogErrors) Inworld::LogError("UInworldMetahumanEditorApi::CanSetupAssetAsInworldMetahuman body mesh needs to be Metahuman");
		return false;
	}

	return true;
}

void UInworldMetahumanEditorApi::SetupAssetAsInworldMetahuman(const FAssetData& AssetData)
{
	if (!CanSetupAssetAsInworldMetahuman(AssetData, true))
	{
		return;
	}

	auto* Object = AssetData.GetAsset();
	auto* Blueprint = Cast<UBlueprint>(Object);

	auto* InworldEditorApi = GetWorld()->GetSubsystem<UInworldEditorApiSubsystem>();

	const UInworldMetahumanEditorSettings* InworldMetahumanEditorSettings = GetDefault<UInworldMetahumanEditorSettings>();

	const TSubclassOf<UInworldCharacterComponent> InworldCharacterComponent = InworldMetahumanEditorSettings->InworldCharacterComponent;
	auto* CharacterComponent = Cast<UInworldCharacterComponent>(InworldEditorApi->AddNodeToBlueprint(Blueprint, InworldCharacterComponent, InworldCharacterComponent->GetName()));
	if (CharacterComponent)
	{
		for (TSubclassOf<UInworldCharacterPlayback> CharacterPlaybackClass : InworldMetahumanEditorSettings->CharacterPlaybacks)
		{
			CharacterComponent->PlaybackTypes.Add(CharacterPlaybackClass);
		}
	}

	for (TSubclassOf<UActorComponent> ActorComponentClass : InworldMetahumanEditorSettings->OtherCharacterComponents)
	{
		InworldEditorApi->AddNodeToBlueprint(Blueprint, ActorComponentClass, ActorComponentClass->GetName());
	}

	auto* FaceComponent = Cast<USkeletalMeshComponent>(InworldEditorApi->GetNodeFromBlueprint(Blueprint, "Face"));
	if (FaceComponent)
	{
		TSoftObjectPtr<UAnimBlueprint> FaceAnimAsset(InworldMetahumanEditorSettings->MetahumanFaceAnimBP);
		FaceAnimAsset.LoadSynchronous();
		FaceComponent->SetAnimInstanceClass(FaceAnimAsset.Get()->GeneratedClass);
	}

	auto* BodyComponent = Cast<USkeletalMeshComponent>(InworldEditorApi->GetNodeFromBlueprint(Blueprint, "Body"));
	if (BodyComponent)
	{
		TSoftObjectPtr<UAnimBlueprint> BodyAnimAsset(InworldMetahumanEditorSettings->MetahumanAnimBP);
		BodyAnimAsset.LoadSynchronous();
		BodyComponent->SetAnimInstanceClass(BodyAnimAsset.Get()->GeneratedClass);
	}
}
