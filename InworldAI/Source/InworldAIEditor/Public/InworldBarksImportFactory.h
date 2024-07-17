// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "Factories/ImportSettings.h"
#include "EditorReimportHandler.h"
#include "InworldBarksImportFactory.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogInworldBarksImportFactory, Log, All);

USTRUCT()
struct FInworldBarkFile
{
	GENERATED_BODY()
public:
	UPROPERTY()
	FString file;

	UPROPERTY()
	FString text;
};

USTRUCT()
struct FInworldBarkJson
{
	GENERATED_BODY()
public:
	UPROPERTY()
	TArray<FInworldBarkFile> files;
};

USTRUCT(BlueprintType)
struct FInworldBarkDataTable : public FTableRowBase
{
	GENERATED_BODY()

	/** Name of the material(s) to search for. Wildcard is supported */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "InworldBarkDataTable")
	FString Text;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MaterialSubstitutionTable")
	TSoftObjectPtr<USoundWave> SoundWaveAssetPtr;
};

UCLASS(hidecategories = Object, MinimalAPI)
class UInworldBarksImportFactory : public UFactory, public IImportSettingsParser
{
	GENERATED_UCLASS_BODY()

public:
	//~ Begin UFactory Interface
	INWORLDAIEDITOR_API virtual bool IsAutomatedImport() const override;
	INWORLDAIEDITOR_API virtual FText GetDisplayName() const override;
	INWORLDAIEDITOR_API virtual UObject* FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags,
		const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled) override;
	INWORLDAIEDITOR_API virtual UObject* FactoryCreateText(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags,
		UObject* Context, const TCHAR* Type, const TCHAR*& Buffer, const TCHAR* BufferEnd, FFeedbackContext* Warn,
		bool& bOutOperationCanceled) override;
	INWORLDAIEDITOR_API virtual bool DoesSupportClass(UClass* Class) override;
	INWORLDAIEDITOR_API virtual bool FactoryCanImport(const FString& Filename) override;
	INWORLDAIEDITOR_API virtual	IImportSettingsParser* GetImportSettingsParser() override;
	INWORLDAIEDITOR_API virtual void CleanUp() override;

	/**
 * IImportSettings interface
 */
	INWORLDAIEDITOR_API virtual void ParseFromJson(TSharedRef<class FJsonObject> ImportSettingsJson) override;
};
