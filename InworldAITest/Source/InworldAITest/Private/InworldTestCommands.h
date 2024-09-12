/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"

#include "InworldSession.h"
#include "InworldCharacter.h"
#include "InworldPackets.h"

#include "Objects/InworldTestObject.h"
#include "InworldAITestSettings.h"

namespace Inworld
{
	namespace Test
	{
		DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(TestCustom, TFunction<bool()>, Func);
		bool TestCustom::Update()
		{
			return Func();
		}

		DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(AddObjectToRoot, UObject*, Object);
		bool AddObjectToRoot::Update()
		{
			Object->AddToRoot();
			return true;
		}

		DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(RemoveObjectFromRoot, UObject*, Object);
		bool RemoveObjectFromRoot::Update()
		{
			Object->RemoveFromRoot();
			GEngine->ForceGarbageCollection(true);
			return true;
		}

		DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(InitSession, UInworldSession*, Session);
		bool InitSession::Update()
		{
			Session->Init();
			return true;
		}

		DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(DestroySession, UInworldSession*, Session);
		bool DestroySession::Update()
		{
			Session->MarkAsGarbage();
			return true;
		}

		DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(StartSessionByScene, UInworldSession*, Session, const FInworldAuth&, Auth, const FString&, SceneId);
		bool StartSessionByScene::Update()
		{
			Session->StartSession({}, Auth, SceneId, {}, {}, {}, {});
			return true;
		}

		DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(StopSession, UInworldSession*, Session);
		bool StopSession::Update()
		{
			Session->StopSession();
			return true;
		}

		DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(WaitUntilSessionConnectingComplete, UInworldSession*, Session);
		bool WaitUntilSessionConnectingComplete::Update()
		{
			const EInworldConnectionState ConnectionState = Session->GetConnectionState();
			return ConnectionState != EInworldConnectionState::Idle && ConnectionState != EInworldConnectionState::Connecting;
		}

		DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(WaitUntilSessionDisconnectingComplete, UInworldSession*, Session);
		bool WaitUntilSessionDisconnectingComplete::Update()
		{
			const EInworldConnectionState ConnectionState = Session->GetConnectionState();
			return ConnectionState != EInworldConnectionState::Connected;
		}

		DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(TestEqualConnectionState, FAutomationTestBase*, Test, UInworldSession*, Session, EInworldConnectionState, ConnectionState);
		bool TestEqualConnectionState::Update()
		{
			static const UEnum* TypeEnum = StaticEnum<EInworldConnectionState>();
			const FName ExpectedTypeName = TypeEnum->GetNameByValue((int64)ConnectionState);
			const FName CurrentTypeName = TypeEnum->GetNameByValue((int64)Session->GetConnectionState());
			Test->TestEqual(TEXT("Connection State"), *CurrentTypeName.ToString(), *ExpectedTypeName.ToString());
			return true;
		}

		DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(WaitUntilSessionLoaded, UInworldSession*, Session);
		bool WaitUntilSessionLoaded::Update()
		{
			return Session->IsLoaded();
		}

		DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(SendCharacterTextMessage, UInworldCharacter*, Character, FString, Text);
		bool SendCharacterTextMessage::Update()
		{
			Character->SendTextMessage(Text);
			return true;
		}

		DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(WaitUntilInteractionEnd, const TArray<FInworldControlEvent>&, ControlEvents);
		bool WaitUntilInteractionEnd::Update()
		{
			return nullptr != ControlEvents.FindByPredicate(
				[](const FInworldControlEvent& ControlEvent) -> bool
				{
					return ControlEvent.Action == EInworldControlEventAction::INTERACTION_END;
				}
			);
		}

#define DEFINE_LATENT_AUTOMATION_COMMAND_CHECK_EVENT_COLLECTION_EMPTY(Type) \
		DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(Test##Type##EventCollection##Empty, FAutomationTestBase*, Test, const TArray<FInworld##Type##Event>&, Type##Events);\
		bool Test##Type##EventCollection##Empty::Update()\
		{\
			Test->TestTrue(FString::Printf(TEXT("%sEvents is empty"), #Type), Type##Events.IsEmpty()); \
			return true;\
		}\
		DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(Test##Type##EventCollection##NotEmpty, FAutomationTestBase*, Test, const TArray<FInworld##Type##Event>&, Type##Events);\
		bool Test##Type##EventCollection##NotEmpty::Update()\
		{\
			Test->TestFalse(FString::Printf(TEXT("%sEvents is empty"), #Type), Type##Events.IsEmpty()); \
			return true;\
		}\

#define DEFINE_LATENT_AUTOMATION_COMMAND_CHECK_EVENT_COLLECTION_VALID(Type) \
		DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(Test##Type##EventCollection##Valid, FAutomationTestBase*, Test, const TArray<FInworld##Type##Event>&, Type##Events);\
		bool Test##Type##EventCollection##Valid::Update()\
		{\
			for(const auto& Type##Event : Type##Events)\
			{\
				Test->TestTrue(FString::Printf(TEXT("%sEvent is valid"), #Type), Type##EventValid(Type##Event)); \
			}\
			return true;\
		}\

		TFunction<bool(const FInworldTextEvent&)> TextEventValid = [](const FInworldTextEvent& TextEvent) -> bool
		{
			return TextEvent.Final && !TextEvent.Text.IsEmpty();
		};
		DEFINE_LATENT_AUTOMATION_COMMAND_CHECK_EVENT_COLLECTION_EMPTY(Text)
		DEFINE_LATENT_AUTOMATION_COMMAND_CHECK_EVENT_COLLECTION_VALID(Text)

		TFunction<bool(const FInworldAudioDataEvent&)> AudioDataEventValid = [](const FInworldAudioDataEvent& AudioDataEvent) -> bool
		{
			return AudioDataEvent.bFinal && !AudioDataEvent.Chunk.IsEmpty();
		};
		DEFINE_LATENT_AUTOMATION_COMMAND_CHECK_EVENT_COLLECTION_EMPTY(AudioData)
		DEFINE_LATENT_AUTOMATION_COMMAND_CHECK_EVENT_COLLECTION_VALID(AudioData)

#undef DEFINE_LATENT_AUTOMATION_COMMAND_CHECK_EVENT_COLLECTION_EMPTY
#undef DEFINE_LATENT_AUTOMATION_COMMAND_CHECK_EVENT_COLLECTION_VALID
	}
}
