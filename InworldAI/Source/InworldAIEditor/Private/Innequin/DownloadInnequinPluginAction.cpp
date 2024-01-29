/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */


#include "DownloadInnequinPluginAction.h"

#include "InworldEditorApi.h"
#include "Developer/DesktopPlatform/Public/IDesktopPlatform.h"
#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"
#include "Misc/MonitoredProcess.h"
#include "Async/Async.h"
#include "Async/TaskGraphInterfaces.h"

UDownloadInnequinPluginAction* UDownloadInnequinPluginAction::DownloadInnequinPlugin(FOnDownloadInnequinLog InLogCallback)
{
	UDownloadInnequinPluginAction* BlueprintNode = NewObject<UDownloadInnequinPluginAction>();
	BlueprintNode->LogCallback = InLogCallback;
	return BlueprintNode;
}

void UDownloadInnequinPluginAction::Activate()
{
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this]()
		{
			bool bSuccess = false;
			if (auto* World = GEditor->GetEditorWorldContext().World())
			{
				auto* InworldEditorApi = World->GetSubsystem<UInworldEditorApiSubsystem>();

				const FString TempPath = FDesktopPlatformModule::Get()->GetUserTempPath();
				const FString ZipLocation = FString::Format(TEXT("{0}{1}"), { TempPath, TEXT("InworldInnequin.zip") });
				const FString ZipURL = FString::Format(TEXT("https://storage.googleapis.com/assets-inworld-ai/models/innequin/unreal/InworldInnequin_{0}.zip"), { InworldEditorApi->GetInnequinVersion() });
				FStringFormatOrderedArguments GetZipFormattedArgs;
				GetZipFormattedArgs.Add(FStringFormatArg(ZipURL));
				GetZipFormattedArgs.Add(FStringFormatArg(TEXT("-o")));
				GetZipFormattedArgs.Add(FStringFormatArg(ZipLocation));
				const FString GetZipCommandLineArgs = FString::Format(TEXT("\"{0}\" {1} \"{2}\""), GetZipFormattedArgs);
				const FString CurlURL =
#if PLATFORM_WINDOWS
					TEXT("curl");
#elif PLATFORM_MAC
					TEXT("/usr/bin/curl");
#endif
				TSharedPtr<FMonitoredProcess> GetZipProcess = MakeShareable(new FMonitoredProcess(CurlURL, GetZipCommandLineArgs, true));
				NotifyLog(TEXT("Downloading..."));
				GetZipProcess->OnOutput().BindLambda([this](const FString& Output)
					{
						NotifyLog(Output);
					}
				);
				if (!GetZipProcess->Launch())
				{
					NotifyComplete(false);
					return;
				}
				while (GetZipProcess->Update())
				{
					FPlatformProcess::Sleep(0.01f);
				}
				const int GetZipRetCode = GetZipProcess->GetReturnCode();
				if (GetZipRetCode != 0)
				{
					NotifyComplete(false);
					return;
				}
				NotifyLog(TEXT("Downloading Complete!"));

				const FString PluginLocation = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*FPaths::ProjectPluginsDir());

				IFileManager::Get().MakeDirectory(*PluginLocation, true);

				FStringFormatOrderedArguments UnzipFormattedArgs;
				UnzipFormattedArgs.Add(FStringFormatArg(TEXT("-xvf")));
				UnzipFormattedArgs.Add(FStringFormatArg(ZipLocation));
				UnzipFormattedArgs.Add(FStringFormatArg(TEXT("-C")));
				UnzipFormattedArgs.Add(FStringFormatArg(PluginLocation));
				const FString UnzipCommandLineArgs = FString::Format(TEXT("{0} \"{1}\" {2} \"{3}\""), UnzipFormattedArgs);
				const FString TarURL =
#if PLATFORM_WINDOWS
					TEXT("tar");
#elif PLATFORM_MAC
					TEXT("/usr/bin/tar");
#endif
				TSharedPtr<FMonitoredProcess> UnzipProcess = MakeShareable(new FMonitoredProcess(TarURL, UnzipCommandLineArgs, true));
				NotifyLog(TEXT("Extracting..."));
				UnzipProcess->OnOutput().BindLambda([this](const FString& Output)
					{
						NotifyLog(Output);
					}
				);
				if (!UnzipProcess->Launch())
				{
					NotifyComplete(false);
					return;
				}
				while (UnzipProcess->Update())
				{
					FPlatformProcess::Sleep(0.01f);
				}
				const int UnzipRetCode = UnzipProcess->GetReturnCode();
				if (UnzipRetCode != 0)
				{
					NotifyComplete(false);
					return;
				}
				NotifyLog(TEXT("Extracting Complete!"));

				NotifyComplete(true);
			}
		}
	);
}

void UDownloadInnequinPluginAction::NotifyLog(const FString& Message)
{
	if (IsInGameThread())
	{
		LogCallback.ExecuteIfBound(Message);
	}
	else
	{
		AsyncTask(ENamedThreads::GameThread, [this, Message]()
			{
				NotifyLog(Message);
			}
		);
	}
}

void UDownloadInnequinPluginAction::NotifyComplete(bool bSuccess)
{
	if (IsInGameThread())
	{
		if (bSuccess)
		{
			if (auto* World = GEditor->GetEditorWorldContext().World())
			{
				auto* InworldEditorApi = World->GetSubsystem<UInworldEditorApiSubsystem>();
				InworldEditorApi->NotifyRestartRequired();
			}
			NotifyLog(TEXT("Restart Required!"));
		}
		else
		{
			NotifyLog(TEXT("Something went wrong! Please download file and install manually."));
		}
		DownloadComplete.Broadcast(bSuccess);
	}
	else
	{
		AsyncTask(ENamedThreads::GameThread, [this, bSuccess]()
			{
				NotifyComplete(bSuccess);
			}
		);
	}
}
