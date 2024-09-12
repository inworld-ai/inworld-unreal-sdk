/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */


#include "InworldBlueprintFunctionLibrary.h"

#include "InworldAIIntegrationSettings.h"

#include "InworldSession.h"
#include "InworldSessionComponent.h"
#include "InworldCharacter.h"
#include "InworldCharacterComponent.h"
#include "InworldPlayer.h"
#include "InworldPlayerComponent.h"

#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "JsonObjectConverter.h"

#include "Interfaces/IPluginManager.h"

#include "Audio.h"
#include "Sound/SoundWave.h"

UInworldSession* UInworldBlueprintFunctionLibrary::Conv_InworldSessionComponentToSession(UInworldSessionComponent* SessionComponent)
{
    return SessionComponent ? IInworldSessionOwnerInterface::Execute_GetInworldSession(SessionComponent) : nullptr;
}

UInworldCharacter* UInworldBlueprintFunctionLibrary::Conv_InworldCharacterComponentToCharacter(UInworldCharacterComponent* CharacterComponent)
{
    return CharacterComponent ? IInworldCharacterOwnerInterface::Execute_GetInworldCharacter(CharacterComponent) : nullptr;
}

UInworldPlayer* UInworldBlueprintFunctionLibrary::Conv_InworldPlayerComponentToPlayer(UInworldPlayerComponent* PlayerComponent)
{
    return PlayerComponent ? IInworldPlayerOwnerInterface::Execute_GetInworldPlayer(PlayerComponent) : nullptr;
}

FString UInworldBlueprintFunctionLibrary::GetInworldAIPluginVersion()
{
    TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin("InworldAI");
    if (Plugin.IsValid())
    {
        return Plugin->GetDescriptor().VersionName;
    }
    return "";
}

FString UInworldBlueprintFunctionLibrary::GetStudioApiKey()
{
    const UInworldAIIntegrationSettings* InworldAIIntegrationSettings = GetDefault<UInworldAIIntegrationSettings>();
    return InworldAIIntegrationSettings->StudioApiKey;
}

template<typename T, typename U>
void GetInworldStudioResource(const U& Callback, const FString& URL, const FString& Auth)
{
    FHttpRequestPtr HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->SetURL(URL);
    HttpRequest->SetVerb("GET");
    HttpRequest->SetHeader("Content-Type", "application/json");
    HttpRequest->SetHeader("Authorization", "Basic " + Auth);
    HttpRequest->SetHeader("Grpc-Metadata-X-Authorization-Bearer-Type", "studio_api");
    HttpRequest->OnProcessRequestComplete().BindLambda(
        [Callback](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
        {
            const int32 ResponseCode = Response->GetResponseCode();
            const FString ResponseString = Response->GetContentAsString();

            T Resource;
            bool bValid = false;
            FString Error;
            if (EHttpResponseCodes::IsOk(Response->GetResponseCode()))
            {
                if (FJsonObjectConverter::JsonObjectStringToUStruct(Response->GetContentAsString(), &Resource))
                {
                    bValid = true;
                }
                else
                {
                    Error = FString::Format(TEXT("Invalid Inworld Studio response. json={0}"), { ResponseString });
                }
            }
            else
            {
                Error = FString::Format(TEXT("Invalid Inworld Studio response. code={0} error={1}"), { FString::FromInt(ResponseCode), ResponseString });
            }
            Callback.ExecuteIfBound(Resource, bValid, Error);
        }
    );
    HttpRequest->ProcessRequest();
}

void UInworldBlueprintFunctionLibrary::GetInworldStudioWorkspaces(const FOnInworldStudioWorkspaces& Callback, const FString& StudioApiKeyOverride)
{
    const FString InworldStudioApiKey = StudioApiKeyOverride.IsEmpty() ? GetStudioApiKey() : StudioApiKeyOverride;
    GetInworldStudioResource<FInworldStudioWorkspaces>(Callback, FString("https://api.inworld.ai/studio/v1/workspaces"), InworldStudioApiKey);
}

void UInworldBlueprintFunctionLibrary::GetInworldStudioApiKeys(const FOnInworldStudioApiKeys& Callback, const FString& Workspace, const FString& StudioApiKeyOverride)
{
    const FString InworldStudioApiKey = StudioApiKeyOverride.IsEmpty() ? GetStudioApiKey() : StudioApiKeyOverride;
    GetInworldStudioResource<FInworldStudioApiKeys>(Callback, FString::Format(TEXT("https://api.inworld.ai/studio/v1/workspaces/{0}/apikeys"), { Workspace }), InworldStudioApiKey);
}

void UInworldBlueprintFunctionLibrary::GetInworldStudioCharacters(const FOnInworldStudioCharacters& Callback, const FString& Workspace, const FString& StudioApiKeyOverride)
{
    const FString InworldStudioApiKey = StudioApiKeyOverride.IsEmpty() ? GetStudioApiKey() : StudioApiKeyOverride;
    GetInworldStudioResource<FInworldStudioCharacters>(Callback, FString::Format(TEXT("https://api.inworld.ai/studio/v1/workspaces/{0}/characters"), { Workspace }), InworldStudioApiKey);
}

void UInworldBlueprintFunctionLibrary::GetInworldStudioScenes(const FOnInworldStudioScenes& Callback, const FString& Workspace, const FString& StudioApiKeyOverride)
{
    const FString InworldStudioApiKey = StudioApiKeyOverride.IsEmpty() ? GetStudioApiKey() : StudioApiKeyOverride;
    GetInworldStudioResource<FInworldStudioScenes>(Callback, FString::Format(TEXT("https://api.inworld.ai/studio/v1/workspaces/{0}/scenes"), { Workspace }), InworldStudioApiKey);
}

bool UInworldBlueprintFunctionLibrary::SoundWaveToDataArray(USoundWave* SoundWave, TArray<uint8>& OutDataArray)
{
    //TODO: support other formats
    constexpr int32 SampleRate = 48000;
    constexpr int32 NumChannels = 2;
    if (!ensure(
        SoundWave->GetSampleRateForCurrentPlatform() == SampleRate &&
        SoundWave->NumChannels == NumChannels
    ))
    {
        return false;
    }

    const uint8* WaveData = SoundWave->RawPCMData;
    const int32 WaveDataSize = SoundWave->RawPCMDataSize;
    constexpr int32 MinSize = 0.01f * SampleRate * NumChannels * sizeof(uint16); // 10ms
    if (!ensure(WaveData && WaveDataSize > MinSize))
    {
        return false;
    }

    OutDataArray.SetNum(WaveDataSize);
    uint8* StrData = (uint8*)OutDataArray.GetData();
    FMemory::Memcpy(StrData, WaveData, WaveDataSize);

    uint16* pDst = (uint16*)OutDataArray.GetData();
    uint16* pSrc = pDst + 5;
    uint16* pEnd = (uint16*)(StrData + OutDataArray.Num());

    // downsample to 16000 per sec and cut the second channel
    while (true)
    {
        FMemory::Memcpy(pDst, pSrc, sizeof(uint16));
        pDst += 1;
        pSrc += 6;
        if (pSrc >= pEnd)
        {
            break;
        }
    }

    const uint32 NewSampleDataSize = (uint8*)pDst - StrData - sizeof(uint16);
    OutDataArray.SetNum(NewSampleDataSize);

    return true;
}

USoundWave* UInworldBlueprintFunctionLibrary::DataArrayToSoundWave(const TArray<uint8>& DataArray)
{
    uint8* Data = (uint8*)DataArray.GetData();
    const uint32 Num = DataArray.Num();

    FWaveModInfo WaveInfo;
    if (!WaveInfo.ReadWaveInfo(Data, Num))
    {
        return nullptr;
    }

    USoundWave* SoundWave = NewObject<USoundWave>(USoundWave::StaticClass());
    if (!ensure(SoundWave))
    {
        return nullptr;
    }

    SoundWave->Duration = *WaveInfo.pWaveDataSize / (*WaveInfo.pChannels * (*WaveInfo.pBitsPerSample / 8.f) * *WaveInfo.pSamplesPerSec);

    SoundWave->SetSampleRate(*WaveInfo.pSamplesPerSec);
    SoundWave->NumChannels = *WaveInfo.pChannels;
    SoundWave->RawPCMDataSize = WaveInfo.SampleDataSize;
    SoundWave->SoundGroup = ESoundGroup::SOUNDGROUP_Voice;
    SoundWave->RawPCMData = (uint8*)FMemory::Malloc(WaveInfo.SampleDataSize);
    FMemory::Memcpy(SoundWave->RawPCMData, WaveInfo.SampleDataStart, WaveInfo.SampleDataSize);

    return SoundWave;
}
