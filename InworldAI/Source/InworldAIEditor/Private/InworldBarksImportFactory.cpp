// Fill out your copyright notice in the Description page of Project Settings.


#include "InworldBarksImportFactory.h"
#include "JsonObjectConverter.h"
#include "PackageTools.h"

DEFINE_LOG_CATEGORY(LogInworldBarksImportFactory);

#define LOCTEXT_NAMESPACE "UInworldBarksImportFactory"

UInworldBarksImportFactory::UInworldBarksImportFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCreateNew = false;
	bEditAfterNew = true;
	SupportedClass = UDataTable::StaticClass();

	bEditorImport = true;
	bText = true;

	Formats.Add(TEXT("json;JavaScript Object Notation"));
}

bool UInworldBarksImportFactory::IsAutomatedImport() const
{
	return bool();
}

FText UInworldBarksImportFactory::GetDisplayName() const
{
	return LOCTEXT("UInworldBarksImportFactoryDescription", "Inworld Barks");
}

UObject* UInworldBarksImportFactory::FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled)
{
	FString FileExtension = FPaths::GetExtension(Filename);
	FString FilePath = FPaths::GetPath(Filename);
	TArray<FString> Path;
	FilePath.ParseIntoArray(Path, TEXT("\\"));
	FString FileDir = Path.Last();

	// load as text
	check(bText); // Set in constructor, so we do not need to support load as binary
	{
		FString Data;
		if (!FFileHelper::LoadFileToString(Data, *Filename, FFileHelper::EHashOptions::None, FILEREAD_AllowWrite))
		{
			UE_LOG(LogInworldBarksImportFactory, Error, TEXT("Failed to load file '%s' to string"), *Filename);
			return nullptr;
		}

		ParseParms(Parms);
		const TCHAR* Buffer = *Data;
		const TCHAR* BufferEnd = *Data + Data.Len();

		FString DataToImport;
		int32 NumChars = UE_PTRDIFF_TO_INT32(BufferEnd - Buffer);
		TArray<TCHAR, FString::AllocatorType>& StringChars = DataToImport.GetCharArray();
		StringChars.AddUninitialized(NumChars + 1);
		FMemory::Memcpy(StringChars.GetData(), Buffer, NumChars * sizeof(TCHAR));
		StringChars.Last() = 0;

		UDataTable* DataTable = NewObject<UDataTable>(InParent, UDataTable::StaticClass(), InName, Flags);
		DataTable->RowStruct = FInworldBarkDataTable::StaticStruct();

		FInworldBarkJson InworldBarkJson;
		if (FJsonObjectConverter::JsonObjectStringToUStruct(DataToImport, &InworldBarkJson))
		{
			FString DestinationPath = InParent->GetPathName();
			DestinationPath.RemoveFromEnd(InName.ToString());
			TArray<FString> ImportFiles;
			for (const FInworldBarkFile& file : InworldBarkJson.files)
			{
				ImportFiles.Add(FilePath + "\\" + file.file);

				FInworldBarkDataTable RowData;
				RowData.Text = file.text;

				FString AssetName = file.file;
				AssetName.RemoveFromEnd(TEXT(".wav"));
				AssetName = UPackageTools::SanitizePackageName(AssetName);

				FString SoundWavePath = "SoundWave'" + DestinationPath + AssetName + "." + AssetName + "'";
				RowData.SoundWaveAssetPtr = FSoftObjectPath(SoundWavePath);
				DataTable->AddRow(FName(file.file), RowData);
			}
			GEditor->GetEditorSubsystem<UImportSubsystem>()->ImportNextTick(ImportFiles, DestinationPath);
		}

		return DataTable;
	}

	return nullptr;
}

UObject* UInworldBarksImportFactory::FactoryCreateText(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, const TCHAR* Type, const TCHAR*& Buffer, const TCHAR* BufferEnd, FFeedbackContext* Warn, bool& bOutOperationCanceled)
{
	return nullptr;
}

bool UInworldBarksImportFactory::DoesSupportClass(UClass* Class)
{
	return bool();
}

bool UInworldBarksImportFactory::FactoryCanImport(const FString& Filename)
{
	return true;
}

IImportSettingsParser* UInworldBarksImportFactory::GetImportSettingsParser()
{
	return this;
}

void UInworldBarksImportFactory::CleanUp()
{
	return void();
}

void UInworldBarksImportFactory::ParseFromJson(TSharedRef<class FJsonObject> ImportSettingsJson)
{
	return void();
}

#undef LOCTEXT_NAMESPACE
