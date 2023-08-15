/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "InworldEditorApi.h"
#include "Kismet/GameplayStatics.h"
#include "InworldCharacterComponent.h"
#include "AssetTools/Public/AssetToolsModule.h"
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

THIRD_PARTY_INCLUDES_START
#include "Utils/Log.h"
THIRD_PARTY_INCLUDES_END

const FString& UInworldEditorApiSubsystem::GetSavedStudioAccessToken() const
{
	const UInworldAIEditorSettings* InworldAIEditorSettings = GetDefault<UInworldAIEditorSettings>();
	return InworldAIEditorSettings->StudioAccessToken;
}

void UInworldEditorApiSubsystem::RequestStudioData(const FString& ExchangeToken)
{
	FInworldEditorClientOptions Options;
	Options.ServerUrl = "api-studio.inworld.ai:443";
	Options.ExchangeToken = ExchangeToken;
	EditorClient->RequestUserData(Options, [this](const FInworldStudioUserData& Data, bool bError)
		{
			CacheStudioData(Data);
			OnLogin.Broadcast(!bError, Data);
		});
}

void UInworldEditorApiSubsystem::CancelRequestStudioData()
{
	EditorClient->CancelRequests();
}

TArray<FString> UInworldEditorApiSubsystem::GetWorldActorNames() const
{
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), Actors);

	TArray<FString> Names;
	Names.Reserve(Actors.Num());
	for (auto* Actor : Actors)
	{
		Names.Add(Actor->GetName());
	}

	return Names;
}

void UInworldEditorApiSubsystem::SetupActor(const FInworldStudioUserCharacterData& Data, const FString& Name, const FString& PreviousName)
{
	if ((Name.IsEmpty() && PreviousName.IsEmpty()) || (Name == PreviousName))
	{
		UE_LOG(LogInworldAIEditor, Error, TEXT("UInworldEditorApiSubsystem::SetupActor invalid names '%s', '%s'"), *Name, *PreviousName);
		return;
	}

	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), Actors);
	
	for (auto* Actor : Actors)
	{
		if (Actor->GetName() == PreviousName)
		{
			if (auto* Component = Actor->GetComponentByClass(UInworldCharacterComponent::StaticClass()))
			{
				Actor->RemoveInstanceComponent(Component);
				continue;
			}
		}

		if (Actor->GetName() == Name)
		{
			auto* Component = NewObject<UInworldCharacterComponent>(Actor);
			if (!Component)
			{
				UE_LOG(LogInworldAIEditor, Error, TEXT("UInworldEditorApiSubsystem::SetupActor couldn't create UInworldCharacterComponent"));
				continue;
			}

			Actor->AddInstanceComponent(Component);
			Component->SetBrainName(Data.Name);
			Component->RegisterComponent();

			if (GEditor)
			{
				FSelectionStateOfLevel SelectionState;
				GEditor->SetSelectionStateOfLevel(SelectionState);

				SelectionState.SelectedActors.Add(Actor->GetPathName());
				GEditor->SetSelectionStateOfLevel(SelectionState);
			}
		}
	}
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

bool UInworldEditorApiSubsystem::DoesSupportWorldType(EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Editor;
}

void UInworldEditorApiSubsystem::Tick(float DeltaTime)
{
	if (EditorClient)
	{
		EditorClient->Tick(DeltaTime);
	}
}

void UInworldEditorApiSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	EditorClient = MakeShared<FInworldEditorClient>();
	EditorClient->Init();

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
}

void UInworldEditorApiSubsystem::Deinitialize()
{
	Super::Deinitialize();

	if (EditorClient)
	{
		EditorClient->Destroy();
	}
	EditorClient.Reset();

	FInworldAIEditorModule& Module = FModuleManager::Get().LoadModuleChecked<FInworldAIEditorModule>("InworldAIEditor");
	Module.UnbindMenuAssetAction(FName("Inworld Player"));
	Module.UnbindMenuAssetAction(FName("Inworld Character"));
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
		UPackage::Save(Package, Object, RF_Public | RF_Standalone, *FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension()));

		AssetRegistry.AssetCreated(Object);

		TArray<UObject*> Objects;
		Objects.Add(Object);
		ContentBrowserModule.Get().SyncBrowserToAssets(Objects);
	}

	return Cast<UBlueprint>(Object);
}
