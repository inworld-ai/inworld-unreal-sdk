/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "InworldEditorApi.h"
#include "Kismet/GameplayStatics.h"
#include "InworldCharacterComponent.h"
#include "AssetToolsModule.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Factories/BlueprintFactory.h"
#include "UObject/UObjectBaseUtility.h"
#include "GameFramework/Character.h"
#include "Engine/SCS_Node.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Animation/AnimBlueprint.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Texture.h"
#include "InworldPlayerComponent.h"
#include "Modules/ModuleManager.h"
#include "InworldAIEditorModule.h"
#include "InworldAIEditorSettings.h"
#include "Templates/Casts.h"
#include "Util/InworldEditorNotification.h"
#include "PluginData/InworldInnequinEditorSettings.h"
#include "PluginData/InworldMetahumanEditorSettings.h"
#include "Interfaces/IPluginManager.h"
#include "UObject/SavePackage.h"

static FString ServerUrl = "api-studio.inworld.ai:443";

FString UInworldEditorApiSubsystem::GetInworldAIPluginVersion()
{
	TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin("InworldAI");
	if (Plugin.IsValid())
	{
		return Plugin->GetDescriptor().VersionName;
	}
	return "";
}

const FString& UInworldEditorApiSubsystem::GetSavedStudioAccessToken() const
{
	const UInworldAIEditorSettings* InworldAIEditorSettings = GetDefault<UInworldAIEditorSettings>();
	return InworldAIEditorSettings->StudioAccessToken;
}

void UInworldEditorApiSubsystem::RequestStudioData(const FString& ExchangeToken)
{
	FInworldEditorClientOptions Options;
	Options.ServerUrl = ServerUrl;
	Options.ExchangeToken = ExchangeToken;
	EditorClient.RequestFirebaseToken(Options, [this](const FString& FirebaseToken)
		{
			Studio.RequestStudioUserData(FirebaseToken, ServerUrl, [this](bool bSuccess) 
				{
					CacheStudioData(Studio.GetStudioUserData());
					OnLogin.Broadcast(bSuccess, Studio.GetStudioUserData());
				}
			);
		});
}

void UInworldEditorApiSubsystem::CancelRequestStudioData()
{
	EditorClient.CancelRequests();
	Studio.CancelRequests();
}

void UInworldEditorApiSubsystem::NotifyRestartRequired()
{
	RestartRequiredNotification->OnRestartRequired();
}

const FInworldStudioUserData& UInworldEditorApiSubsystem::GetCachedStudioData() const
{
	FInworldAIEditorModule* Module = static_cast<FInworldAIEditorModule*>(FModuleManager::Get().GetModule("InworldAIEditor"));
	if (ensure(Module))
	{
		return Module->GetStudioData();
	}
	static FInworldStudioUserData Data;
	return Data;
}

void UInworldEditorApiSubsystem::BindActionForCharacterData(const FName& Name, FOnCharacterStudioDataPermission Permission, FOnCharacterStudioDataAction Action)
{
	FCharacterStudioDataFunctions FunctionsForName;
	FunctionsForName.Permission = Permission;
	FunctionsForName.Action = Action;
	CharacterStudioDataFunctionMap.Add(Name, FunctionsForName);
}

void UInworldEditorApiSubsystem::UnbindActionForCharacterData(const FName& Name)
{
	CharacterStudioDataFunctionMap.Remove(Name);
}

void UInworldEditorApiSubsystem::GetCharacterDataActions(TArray<FName>& OutKeys) const
{
	return CharacterStudioDataFunctionMap.GenerateKeyArray(OutKeys);
}

bool UInworldEditorApiSubsystem::CanExecuteCharacterDataAction(const FName& Name, const FInworldStudioUserCharacterData& CharacterStudioData)
{
	if (ensure(CharacterStudioDataFunctionMap.Contains(Name)))
	{
		return CharacterStudioDataFunctionMap[Name].Permission.Execute(CharacterStudioData);
	}
	return false;
}

void UInworldEditorApiSubsystem::ExecuteCharacterDataAction(const FName& Name, const FInworldStudioUserCharacterData& CharacterStudioData)
{
	if (ensure(CharacterStudioDataFunctionMap.Contains(Name)) && CanExecuteCharacterDataAction(Name, CharacterStudioData))
	{
		CharacterStudioDataFunctionMap[Name].Action.ExecuteIfBound(CharacterStudioData);
	}
}

bool UInworldEditorApiSubsystem::CanSetupAssetAsInworldPlayer(const FAssetData& AssetData, bool bLogErrors)
{
	auto* Object = AssetData.GetAsset();
	if (!Object)
	{
		if (bLogErrors) UE_LOG(LogInworldAIEditor, Error, TEXT("UInworldEditorApiSubsystem::CanSetupAssetAsInworldPlayer couldn't find Object"));
		return false;
	}

	auto* Blueprint = Cast<UBlueprint>(Object);
	if (!Blueprint || !Blueprint->SimpleConstructionScript)
	{
		if (bLogErrors) UE_LOG(LogInworldAIEditor, Error, TEXT("UInworldEditorApiSubsystem::CanSetupAssetAsInworldPlayer asset should be Blueprint with SimpleConstructionScript"));
		return false;
	}

	const UInworldAIEditorSettings* InworldAIEditorSettings = GetDefault<UInworldAIEditorSettings>();

	if (!ensure(InworldAIEditorSettings->InworldPlayerComponent))
	{
		if(bLogErrors) UE_LOG(LogInworldAIEditor, Error, TEXT("UInworldEditorApiSubsystem::CanSetupAssetAsInworldPlayer InworldPlayerComponent is nullptr"));
		return false;
	}

	return true;
}

void UInworldEditorApiSubsystem::SetupAssetAsInworldPlayer(const FAssetData& AssetData)
{
	if (!CanSetupAssetAsInworldPlayer(AssetData, true))
	{
		return;
	}

	auto* Object = AssetData.GetAsset();
	auto* Blueprint = Cast<UBlueprint>(Object);
	SetupBlueprintAsInworldPlayer(Blueprint);
}

void UInworldEditorApiSubsystem::SetupBlueprintAsInworldPlayer(UBlueprint* Blueprint)
{
	const UInworldAIEditorSettings* InworldAIEditorSettings = GetDefault<UInworldAIEditorSettings>();

	const TSubclassOf<UInworldPlayerComponent> InworldPlayerComponent = InworldAIEditorSettings->InworldPlayerComponent;
	AddNodeToBlueprint(Blueprint, InworldPlayerComponent, InworldPlayerComponent->GetName());

	for (TSubclassOf<UActorComponent> ActorComponentClass : InworldAIEditorSettings->OtherPlayerComponents)
	{
		AddNodeToBlueprint(Blueprint, ActorComponentClass, ActorComponentClass->GetName());
	}
}

bool UInworldEditorApiSubsystem::CanSetupAssetAsInworldCharacter(const FAssetData& AssetData, bool bLogErrors)
{
	auto* Object = AssetData.GetAsset();
	if (!Object)
	{
		if (bLogErrors) UE_LOG(LogInworldAIEditor, Error, TEXT("UInworldEditorApiSubsystem::CanSetupAssetAsInworldCharacter couldn't find Object"));
		return false;
	}

	auto* Blueprint = Cast<UBlueprint>(Object);
	if (!Blueprint || !Blueprint->SimpleConstructionScript)
	{
		if (bLogErrors) UE_LOG(LogInworldAIEditor, Error, TEXT("UInworldEditorApiSubsystem::CanSetupAssetAsInworldCharacter asset should be Blueprint with SimpleConstructionScript"));
		return false;
	}

	const UInworldAIEditorSettings* InworldAIEditorSettings = GetDefault<UInworldAIEditorSettings>();

	if (!ensure(InworldAIEditorSettings->InworldCharacterComponent))
	{
		if (bLogErrors) UE_LOG(LogInworldAIEditor, Error, TEXT("UInworldEditorApiSubsystem::CanSetupAssetAsInworldCharacter InworldCharacterComponent is nullptr"));
		return false;
	}

	return true;
}

void UInworldEditorApiSubsystem::SetupAssetAsInworldCharacter(const FAssetData& AssetData)
{
	if (!CanSetupAssetAsInworldCharacter(AssetData, true))
	{
		return;
	}

	auto* Object = AssetData.GetAsset();
	auto* Blueprint = Cast<UBlueprint>(Object);

	SetupBlueprintAsInworldCharacter(Blueprint);
}

void UInworldEditorApiSubsystem::SetupBlueprintAsInworldCharacter(UBlueprint* Blueprint)
{
	const UInworldAIEditorSettings* InworldAIEditorSettings = GetDefault<UInworldAIEditorSettings>();

	const TSubclassOf<UInworldCharacterComponent> InworldCharacterComponent = InworldAIEditorSettings->InworldCharacterComponent;
	auto* CharacterComponent = Cast<UInworldCharacterComponent>(AddNodeToBlueprint(Blueprint, InworldCharacterComponent, InworldCharacterComponent->GetName()));
	if (CharacterComponent)
	{
		for (TSubclassOf<UInworldCharacterPlayback> CharacterPlaybackClass : InworldAIEditorSettings->CharacterPlaybacks)
		{
			CharacterComponent->PlaybackTypes.Add(CharacterPlaybackClass);
		}
	}

	for (TSubclassOf<UActorComponent> ActorComponentClass : InworldAIEditorSettings->OtherCharacterComponents)
	{
		AddNodeToBlueprint(Blueprint, ActorComponentClass, ActorComponentClass->GetName());
	}
}

bool UInworldEditorApiSubsystem::CanSetupAssetAsInworldMetahuman(const FAssetData& AssetData, bool bLogErrors)
{
	auto* Object = AssetData.GetAsset();
	if (!Object)
	{
		if (bLogErrors) UE_LOG(LogInworldAIEditor, Error, TEXT("UInworldEditorApiSubsystem::CanSetupAssetAsInworldMetahuman couldn't find Object"));
		return false;
	}

	auto* Blueprint = Cast<UBlueprint>(Object);
	if (!Blueprint || !Blueprint->SimpleConstructionScript)
	{
		if (bLogErrors) UE_LOG(LogInworldAIEditor, Error, TEXT("UInworldEditorApiSubsystem::CanSetupAssetAsInworldMetahuman asset should be Blueprint with SimpleConstructionScript"));
		return false;
	}

	const UInworldMetahumanEditorSettings* InworldMetahumanEditorSettings = GetDefault<UInworldMetahumanEditorSettings>();

	auto* FaceComponent = Cast<USkeletalMeshComponent>(GetNodeFromBlueprint(Blueprint, "Face"));
	if (!FaceComponent)
	{
		if (bLogErrors) UE_LOG(LogInworldAIEditor, Error, TEXT("UInworldEditorApiSubsystem::CanSetupAssetAsInworldMetahuman asset should contain a Face Skeletal Mesh"));
		return false;
	}

	if (InworldMetahumanEditorSettings->MetahumanFaceSkeleton.LoadSynchronous() == nullptr)
	{
		if (bLogErrors) UE_LOG(LogInworldAIEditor, Error, TEXT("UInworldEditorApiSubsystem::CanSetupAssetAsInworldMetahuman needs a valid Face Skeleton Asset"));
		return false;
	}

	if (InworldMetahumanEditorSettings->MetahumanFaceABP.LoadSynchronous() == nullptr)
	{
		if (bLogErrors) UE_LOG(LogInworldAIEditor, Error, TEXT("UInworldEditorApiSubsystem::CanSetupAssetAsInworldMetahuman needs a valid Face Anim Asset"));
		return false;
	}

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION > 0
	USkeletalMesh* FaceMesh = FaceComponent->GetSkeletalMeshAsset();
#else
	USkeletalMesh* FaceMesh = FaceComponent->SkeletalMesh;
#endif
	if (!FaceMesh || FaceMesh->GetSkeleton() != InworldMetahumanEditorSettings->MetahumanFaceSkeleton.LoadSynchronous())
	{
		if (bLogErrors) UE_LOG(LogInworldAIEditor, Error, TEXT("UInworldEditorApiSubsystem::CanSetupAssetAsInworldMetahuman face skeleton needs to be Metahuman"));
		return false;
	}

	auto* BodyComponent = Cast<USkeletalMeshComponent>(GetNodeFromBlueprint(Blueprint, "Body"));
	if (!BodyComponent)
	{
		if (bLogErrors) UE_LOG(LogInworldAIEditor, Error, TEXT("UInworldEditorApiSubsystem::CanSetupAssetAsInworldMetahuman asset should contain a Body Skeletal Mesh"));
		return false;
	}

	if (InworldMetahumanEditorSettings->MetahumanBodySkeleton.LoadSynchronous() == nullptr)
	{
		if (bLogErrors) UE_LOG(LogInworldAIEditor, Error, TEXT("UInworldEditorApiSubsystem::CanSetupAssetAsInworldMetahuman needs a valid Body Skeleton Asset"));
		return false;
	}

	if (InworldMetahumanEditorSettings->MetahumanBodyABP.LoadSynchronous() == nullptr)
	{
		if (bLogErrors) UE_LOG(LogInworldAIEditor, Error, TEXT("UInworldEditorApiSubsystem::CanSetupAssetAsInworldMetahuman needs a valid Body Anim Asset"));
		return false;
	}

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION > 0
	USkeletalMesh* BodyMesh = BodyComponent->GetSkeletalMeshAsset();
#else
	USkeletalMesh* BodyMesh = BodyComponent->SkeletalMesh;
#endif
	if (!BodyMesh || BodyMesh->GetSkeleton() != InworldMetahumanEditorSettings->MetahumanBodySkeleton.LoadSynchronous())
	{
		if (bLogErrors) UE_LOG(LogInworldAIEditor, Error, TEXT("UInworldEditorApiSubsystem::CanSetupAssetAsInworldMetahuman body skeleton needs to be Metahuman"));
		return false;
	}
	return true;
}

void UInworldEditorApiSubsystem::SetupAssetAsInworldMetahuman(const FAssetData& AssetData)
{
	if (!CanSetupAssetAsInworldMetahuman(AssetData, true))
	{
		return;
	}

	auto* Object = AssetData.GetAsset();
	auto* Blueprint = Cast<UBlueprint>(Object);

	SetupBlueprintAsInworldMetahuman(Blueprint);
}

void UInworldEditorApiSubsystem::SetupBlueprintAsInworldMetahuman(UBlueprint* Blueprint)
{
	const UInworldMetahumanEditorSettings* InworldMetahumanEditorSettings = GetDefault<UInworldMetahumanEditorSettings>();

	SetupBlueprintAsInworldCharacter(Blueprint);

	auto* CharacterComponent = Cast<UInworldCharacterComponent>(GetNodeFromBlueprint(Blueprint, TEXT("InworldCharacterComponent")));
	if (!CharacterComponent)
	{
		UE_LOG(LogInworldAIEditor, Error, TEXT("UInworldEditorApiSubsystem::CreateInnequinActor couldn't find UInworldCharacterComponent"));
		return;
	}

	for (TSubclassOf<UInworldCharacterPlayback> CharacterPlaybackClass : InworldMetahumanEditorSettings->CharacterPlaybacks)
	{
		CharacterComponent->PlaybackTypes.Add(CharacterPlaybackClass);
	}

	for (TSubclassOf<UActorComponent> ActorComponentClass : InworldMetahumanEditorSettings->CharacterComponents)
	{
		AddNodeToBlueprint(Blueprint, ActorComponentClass, ActorComponentClass->GetName());
	}

	auto* FaceComponent = Cast<USkeletalMeshComponent>(GetNodeFromBlueprint(Blueprint, "Face"));
	if (FaceComponent)
	{
		FaceComponent->SetAnimInstanceClass(InworldMetahumanEditorSettings->MetahumanFaceABP.LoadSynchronous()->GeneratedClass);
	}

	auto* BodyComponent = Cast<USkeletalMeshComponent>(GetNodeFromBlueprint(Blueprint, "Body"));
	if (BodyComponent)
	{
		BodyComponent->SetAnimInstanceClass(InworldMetahumanEditorSettings->MetahumanBodyABP.LoadSynchronous()->GeneratedClass);
	}
}

void UInworldEditorApiSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	RestartRequiredNotification = MakeShared<FInworldEditorRestartRequiredNotification>();
	RestartRequiredNotification->SetOnRestartApplicationCallback(FSimpleDelegate::CreateLambda([]()
		{
			FUnrealEdMisc::Get().RestartEditor(false);
		}
	));

	EditorClient.Init();

	FInworldAIEditorModule& Module = FModuleManager::Get().LoadModuleChecked<FInworldAIEditorModule>("InworldAIEditor");
	Module.BindMenuAssetAction(
		FName("Inworld Player"),
		FName("Player"),
		FText::FromString("Setup as Inworld Player"),
		FText::FromString("Setup this Actor as Inworld Player"),
		FAssetAction::CreateUObject(this, &UInworldEditorApiSubsystem::SetupAssetAsInworldPlayer),
		FAssetActionPermission::CreateUObject(this, &UInworldEditorApiSubsystem::CanSetupAssetAsInworldPlayer, false)
	);
	Module.BindMenuAssetAction(
		FName("Inworld Character"),
		FName("Character"),
		FText::FromString("Setup as Inworld Character"),
		FText::FromString("Setup this Actor as Inworld Character"),
		FAssetAction::CreateUObject(this, &UInworldEditorApiSubsystem::SetupAssetAsInworldCharacter),
		FAssetActionPermission::CreateUObject(this, &UInworldEditorApiSubsystem::CanSetupAssetAsInworldCharacter, false)
	);
	if (IPluginManager::Get().FindPlugin("InworldMetahuman").IsValid())
	{
		Module.BindMenuAssetAction(
			FName("Inworld Metahuman"),
			FName("Character"),
			FText::FromString("Setup as Inworld Metahuman"),
			FText::FromString("Setup this Actor as Inworld Metahuman"),
			FAssetAction::CreateUObject(this, &UInworldEditorApiSubsystem::SetupAssetAsInworldMetahuman),
			FAssetActionPermission::CreateUObject(this, &UInworldEditorApiSubsystem::CanSetupAssetAsInworldMetahuman, false)
		);
	}

	FOnCharacterStudioDataPermission PermissionDelegate;
	PermissionDelegate.BindDynamic(this, &UInworldEditorApiSubsystem::CanCreateInnequinActor);
	FOnCharacterStudioDataAction ActionDelegate;
	ActionDelegate.BindDynamic(this, &UInworldEditorApiSubsystem::CreateInnequinActor);
	BindActionForCharacterData(FName("Create Inworld Avatar"), PermissionDelegate, ActionDelegate);
}

void UInworldEditorApiSubsystem::Deinitialize()
{
	Super::Deinitialize();

	EditorClient.Destroy();

	FInworldAIEditorModule& Module = FModuleManager::Get().LoadModuleChecked<FInworldAIEditorModule>("InworldAIEditor");
	Module.UnbindMenuAssetAction(FName("Inworld Player"));
	Module.UnbindMenuAssetAction(FName("Inworld Character"));
	if (IPluginManager::Get().FindPlugin("InworldMetahuman").IsValid())
	{
		Module.UnbindMenuAssetAction(FName("Inworld Metahuman"));
	}

	UnbindActionForCharacterData(FName("Create Inworld Avatar"));
}

void UInworldEditorApiSubsystem::SavePackageToCharacterFolder(UObject* Object, const FInworldStudioUserCharacterData& CharacterData, const FString& NamePrefix, FString NameSuffix)
{
	FAssetToolsModule& AssetToolsModule = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools");

	FString Name, PackageName;
	const FString Path = FString::Printf(TEXT("/Game/Inworld/%s/%s_%s%s"), *CharacterData.Name, *NamePrefix, *CharacterData.ShortName, *NameSuffix);
	AssetToolsModule.Get().CreateUniqueAssetName(Path, TEXT(""), PackageName, Name);
	const FString PackagePath = FPackageName::GetLongPackagePath(PackageName);

	TArray<FAssetRenameData> Assets;
	FAssetRenameData& AssetData = Assets.Emplace_GetRef();
	AssetData.Asset = Object;
	AssetData.NewPackagePath = PackagePath;
	AssetData.NewName = Name;

	AssetToolsModule.Get().RenameAssets(Assets);
}

UObject* UInworldEditorApiSubsystem::AddNodeToBlueprint(UBlueprint* Blueprint, UClass* Class, const FString& NodeName)
{
	USCS_Node* BPNode = Blueprint->SimpleConstructionScript->CreateNode(Class, *NodeName);
	Blueprint->SimpleConstructionScript->AddNode(BPNode);
	FKismetEditorUtilities::CompileBlueprint(Blueprint);
	return BPNode->ComponentTemplate;
}

UObject* UInworldEditorApiSubsystem::AddNodeToBlueprintNode(UBlueprint* Blueprint, const FString& ParentNodeName, UClass* Class, const FString& NodeName)
{
	USCS_Node* BPNodeParent = Blueprint->SimpleConstructionScript->FindSCSNode(*ParentNodeName);
	if (BPNodeParent)
	{
		USCS_Node* BPNodeChild = Blueprint->SimpleConstructionScript->CreateNode(Class, *NodeName);
		BPNodeParent->AddChildNode(BPNodeChild);
		FKismetEditorUtilities::CompileBlueprint(Blueprint);
		return BPNodeChild->ComponentTemplate;
	}
	return nullptr;
}

UObject* UInworldEditorApiSubsystem::GetNodeFromBlueprint(UBlueprint* Blueprint, const FString& NodeName)
{
	USCS_Node* BPNode = Blueprint->SimpleConstructionScript->FindSCSNode(*NodeName);
	if (BPNode)
	{
		return BPNode->ComponentTemplate;
	}
	return nullptr;
}

void UInworldEditorApiSubsystem::CacheStudioData(const FInworldStudioUserData& Data)
{
	FInworldAIEditorModule* Module = static_cast<FInworldAIEditorModule*>(FModuleManager::Get().GetModule("InworldAIEditor"));
	if (ensure(Module))
	{
		Module->SetStudioData(Data);
	}
}

UBlueprint* UInworldEditorApiSubsystem::CreateCharacterActorBP(const FInworldStudioUserCharacterData& CharacterData)
{
	FAssetToolsModule& AssetToolsModule = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools");
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	FString Name, PackageName;
	const FString Path = FString::Printf(TEXT("/Game/Inworld/%s/BP_%s"), *CharacterData.Name, *CharacterData.ShortName);
	AssetToolsModule.Get().CreateUniqueAssetName(Path, TEXT(""), PackageName, Name);
	const FString PackagePath = FPackageName::GetLongPackagePath(PackageName);

	UPackage* Package = CreatePackage(*PackageName);
	UBlueprintFactory* Factory = NewObject<UBlueprintFactory>(UBlueprintFactory::StaticClass());
	UObject* Object = AssetToolsModule.Get().CreateAsset(Name, PackagePath, UBlueprint::StaticClass(), Factory);
	if (Object)
	{
		FString FileName = FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());
#if ENGINE_MAJOR_VERSION >= 5
		FSavePackageArgs SavePackageArgs;
		SavePackageArgs.TopLevelFlags = RF_Public | RF_Standalone;
		UPackage::SavePackage(Package, Object, *FileName, SavePackageArgs);
#else
		UPackage::SavePackage(Package, Object, RF_Public | RF_Standalone, *FileName);
#endif

		AssetRegistry.AssetCreated(Object);

		TArray<UObject*> Objects;
		Objects.Add(Object);
		ContentBrowserModule.Get().SyncBrowserToAssets(Objects);
	}

	return Cast<UBlueprint>(Object);
}

bool UInworldEditorApiSubsystem::CanCreateInnequinActor(const FInworldStudioUserCharacterData& CharacterData)
{
	TSharedPtr<IPlugin> InworldInnequinPlugin = IPluginManager::Get().FindPlugin("InworldInnequin");
	return InworldInnequinPlugin.IsValid();
}

void UInworldEditorApiSubsystem::CreateInnequinActor(const FInworldStudioUserCharacterData& CharacterData)
{
	const UInworldInnequinEditorSettings* InworldInnequinEditorSettings = GetDefault<UInworldInnequinEditorSettings>();

	UBlueprint* Blueprint = CreateCharacterActorBP(CharacterData);
	auto* MeshComponent = Cast<USkeletalMeshComponent>(AddNodeToBlueprint(Blueprint, USkeletalMeshComponent::StaticClass(), TEXT("Mesh")));
	if (!MeshComponent)
	{
		UE_LOG(LogInworldAIEditor, Error, TEXT("UInworldEditorApiSubsystem::CreateInnequinActor couldn't create USkeletalMeshComponent"));
		return;
	}

	MeshComponent->SetSkeletalMesh(InworldInnequinEditorSettings->InnequinMesh.LoadSynchronous());
	MeshComponent->SetAnimInstanceClass(InworldInnequinEditorSettings->InnequinABP.LoadSynchronous()->GeneratedClass);

	SetupBlueprintAsInworldCharacter(Blueprint);

	auto* CharacterComponent = Cast<UInworldCharacterComponent>(GetNodeFromBlueprint(Blueprint, TEXT("InworldCharacterComponent")));
	if (!CharacterComponent)
	{
		UE_LOG(LogInworldAIEditor, Error, TEXT("UInworldEditorApiSubsystem::CreateInnequinActor couldn't find UInworldCharacterComponent"));
		return;
	}

	CharacterComponent->SetBrainName(CharacterData.Name);
	for (TSubclassOf<UInworldCharacterPlayback> CharacterPlaybackClass : InworldInnequinEditorSettings->CharacterPlaybacks)
	{
		CharacterComponent->PlaybackTypes.Add(CharacterPlaybackClass);
	}

	for (TSubclassOf<UActorComponent> ActorComponentClass : InworldInnequinEditorSettings->CharacterComponents)
	{
		AddNodeToBlueprint(Blueprint, ActorComponentClass, ActorComponentClass->GetName());
	}

	auto* EmoteComponent = Cast<USceneComponent>(GetNodeFromBlueprint(Blueprint, TEXT("PaperSpriteComponent")));

	EmoteComponent->SetupAttachment(MeshComponent, TEXT("EmoteSocket"));
	if (USCS_Node* SCS_Node = Blueprint->SimpleConstructionScript->FindSCSNode(TEXT("PaperSpriteComponent")))
	{
		SCS_Node->Modify();
		SCS_Node->AttachToName = FName("EmoteSocket");
	}
	EmoteComponent->SetRelativeScale3D(FVector(0.0475f, 0.0475f, 0.0475f));
	EmoteComponent->SetRelativeRotation(FRotator(0.f, 0.f, -90.f));

	FKismetEditorUtilities::CompileBlueprint(Blueprint);
	SavePackageToCharacterFolder(Blueprint, CharacterData, "BP");
}
