/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldRPMEditorApi.h"
#include "InworldRPMEditorModule.h"
#include <Engine/Engine.h>
#include "InworldEditorApi.h"
#include "InworldRPMEditorSettings.h"
#include "glTFRuntimeParser.h"
#include "glTFRuntimeAsset.h"
#include "glTFRuntimeFunctionLibrary.h"
#include "InworldCharacterComponent.h"
#include <Engine/Texture2D.h>
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 2
#include <Engine/SkinnedAssetCommon.h>
#endif

THIRD_PARTY_INCLUDES_START
#include "Utils/Log.h"
THIRD_PARTY_INCLUDES_END

void UInworldRPMEditorApi::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
    auto* InworldEditorApi = GetWorld()->GetSubsystem<UInworldEditorApiSubsystem>();
    if (InworldEditorApi)
    {
		FOnCharacterStudioDataPermission PermissionDelegate;
		PermissionDelegate.BindDynamic(this, &UInworldRPMEditorApi::CanCreateReadyPlayerMeActor);
        FOnCharacterStudioDataAction ActionDelegate;
        ActionDelegate.BindDynamic(this, &UInworldRPMEditorApi::CreateReadyPlayerMeActor);
        InworldEditorApi->BindActionForCharacterData(FName("Ready Player Me"), PermissionDelegate, ActionDelegate);
    }
}

void UInworldRPMEditorApi::Deinitialize()
{
	Super::Deinitialize();
	auto* InworldEditorApi = GetWorld()->GetSubsystem<UInworldEditorApiSubsystem>();
	if (InworldEditorApi)
	{
		InworldEditorApi->UnbindActionForCharacterData(FName("Ready Player Me"));
	}
}

bool UInworldRPMEditorApi::DoesSupportWorldType(EWorldType::Type WorldType) const
{
    return WorldType == EWorldType::Editor;
}

void UInworldRPMEditorApi::CreateReadyPlayerMeActor(const FInworldStudioUserCharacterData& CharacterData)
{
	if (!ensure(CanCreateReadyPlayerMeActor(CharacterData)))
	{
		return;
	}

    auto* InworldEditorApi = GetWorld()->GetSubsystem<UInworldEditorApiSubsystem>();
	if (InworldEditorApi)
	{
		InworldEditorApi->EditorClient->RequestReadyPlayerMeModelData(CharacterData, [this, CharacterData](const TArray<uint8>& Data)
		{
				FglTFRuntimeConfig Cfg;
				Cfg.TransformBaseType = EglTFRuntimeTransformBaseType::YForward;
				glTFRuntimeAsset = UglTFRuntimeFunctionLibrary::glTFLoadAssetFromData(Data, Cfg);
				if (glTFRuntimeAsset)
				{
					if (glTFRuntimeAsset->GetNodes().ContainsByPredicate(
						[](const FglTFRuntimeNode& Node) -> bool
						{
							return Node.MeshIndex != -1 && Node.SkinIndex != -1;
						}))
					{

						auto* InworldEditorApi = GetWorld()->GetSubsystem<UInworldEditorApiSubsystem>();
						const UInworldRPMEditorSettings* InworldRPMEditorSettings = GetDefault<UInworldRPMEditorSettings>();

						// load skeletal mesh
						TSoftObjectPtr<USkeleton> RPMSkeleton(InworldRPMEditorSettings->RPMSkeleton);
						RPMSkeleton.LoadSynchronous();

						FglTFRuntimeSkeletalMeshConfig Config;
						Config.Skeleton = RPMSkeleton.Get();
						Config.SkeletonConfig.bAddRootBone = true;
						Config.SkeletonConfig.RootBoneName = "Armature";
						Config.SkeletonConfig.CopyRotationsFrom = RPMSkeleton.Get();

						RuntimeSkeletalMesh = glTFRuntimeAsset->LoadSkeletalMeshRecursive("Armature", {}, Config);
						if (!RuntimeSkeletalMesh)
						{
							UE_LOG(LogInworldRPMEditor, Error, TEXT("UInworldEditorApiSubsystem::CreateRPMActor couldn't load Skeletal Mesh."));
							return;
						}

						// save loaded materials
						int32 MatIdx = 0;
						for (auto& MaterialStruct : RuntimeSkeletalMesh->GetMaterials())
						{
							auto* MatInstance = Cast<UMaterialInstanceDynamic>(MaterialStruct.MaterialInterface);
							if (!ensure(MatInstance))
							{
								continue;
							}

							InworldEditorApi->SavePackageToCharacterFolder(MatInstance, CharacterData, "M", FString::FromInt(MatIdx++));

							int32 TexIdx = 0;
							for (auto& TextureVal : MatInstance->TextureParameterValues)
							{
								// save texture resource
								UTexture2D* Texture = nullptr;
#if ENGINE_MAJOR_VERSION == 4
								Texture = Cast<UTexture2D>(TextureVal.ParameterValue);
#else
								Texture = Cast<UTexture2D>(TextureVal.ParameterValue.Get());
#endif
								auto Mip = Texture->GetPlatformMips()[0];

								uint8* TexData = (uint8*)Mip.BulkData.Lock(LOCK_READ_ONLY);
								Texture->Source.Init(Mip.SizeX, Mip.SizeY, 1, 1, ETextureSourceFormat::TSF_BGRA8, TexData);
								Mip.BulkData.Unlock();

#undef UpdateResource
								Texture->UpdateResource();

								InworldEditorApi->SavePackageToCharacterFolder(Texture, CharacterData, "T", FString::Printf(TEXT("%d%d"), MatIdx, TexIdx++));
							}
						}

						InworldEditorApi->SavePackageToCharacterFolder(RuntimeSkeletalMesh, CharacterData, "SM");

						// create Actor BP
						RuntimeActorBP = InworldEditorApi->CreateCharacterActorBP(CharacterData);
						if (!RuntimeActorBP)
						{
							UE_LOG(LogInworldRPMEditor, Error, TEXT("UInworldEditorApiSubsystem::CreateReadyPlayerMeActor couldn't create RuntimeActorBP."));
							return;
						}

						// create and setup USkeletalMeshComponent
						auto* MeshComponent = Cast<USkeletalMeshComponent>(InworldEditorApi->AddNodeToBlueprint(RuntimeActorBP, USkeletalMeshComponent::StaticClass(), TEXT("SkeletalMeshComponent")));
						if (!MeshComponent)
						{
							UE_LOG(LogInworldRPMEditor, Error, TEXT("UInworldEditorApiSubsystem::CreateReadyPlayerMeActor couldn't create USkeletalMeshComponent"));
							return;
						}

						MeshComponent->SkeletalMesh = RuntimeSkeletalMesh;

						// setup Anim BP
						TSoftObjectPtr<UAnimBlueprint> RPMAnimBP(InworldRPMEditorSettings->RPMAnimBP);
						RPMAnimBP.LoadSynchronous();
						MeshComponent->SetAnimInstanceClass(RPMAnimBP->GeneratedClass);

						// create and setup UInworldCharacterComponent
						auto* InworldComponent = Cast<UInworldCharacterComponent>(InworldEditorApi->AddNodeToBlueprint(RuntimeActorBP, InworldRPMEditorSettings->InworldCharacterComponent, TEXT("InworldCharacterComponent")));
						if (!InworldComponent)
						{
							UE_LOG(LogInworldRPMEditor, Error, TEXT("UInworldEditorApiSubsystem::CreateReadyPlayerMeActor couldn't create UInworldCharacterComponent"));
							return;
						}

						InworldComponent->SetBrainName(CharacterData.Name);

						for (auto PlaybackType : InworldRPMEditorSettings->CharacterPlaybacks)
						{
							InworldComponent->PlaybackTypes.Add(PlaybackType);
						}

						for (auto ActorComponentClass : InworldRPMEditorSettings->OtherCharacterComponents)
						{
							InworldEditorApi->AddNodeToBlueprint(RuntimeActorBP, ActorComponentClass, ActorComponentClass->GetName());
						}
					}
				}
		});
    }
}

bool UInworldRPMEditorApi::CanCreateReadyPlayerMeActor(const FInworldStudioUserCharacterData& CharacterData)
{
	if (CharacterData.RpmModelUri.IsEmpty())
	{
		return false;
	}

	const UInworldRPMEditorSettings* InworldRPMEditorSettings = GetDefault<UInworldRPMEditorSettings>();

	TSoftObjectPtr<USkeleton> RPMSkeleton(InworldRPMEditorSettings->RPMSkeleton);
	RPMSkeleton.LoadSynchronous();

	if (!RPMSkeleton.IsValid())
	{
		return false;
	}

	TSoftObjectPtr<UAnimBlueprint> RPMAnimBP(InworldRPMEditorSettings->RPMAnimBP);
	RPMAnimBP.LoadSynchronous();

	if (!RPMAnimBP.IsValid())
	{
		return false;
	}

	return true;
}
