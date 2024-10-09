/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "Commands/InworldTestCommandsSession.h"
#include "InworldTestMacros.h"
#include "InworldTestChecks.h"

#include "InworldSession.h"

bool Inworld::Test::FInitSpeechProcessorCommand::Update()
{
	Session->InitSpeechProcessor(Mode, SpeechOptions);
	return true;
}

bool Inworld::Test::FDestroySpeechProcessorCommand::Update()
{
	Session->DestroySpeechProcessor();
	return true;
}

bool Inworld::Test::FStartSessionBySceneCommand::Update()
{
	Session->StartSessionFromScene(Scene, {}, {}, {}, Workspace, Auth);
	return true;
}

bool Inworld::Test::FStopSessionCommand::Update()
{
	Session->StopSession();
	return true;
}

bool Inworld::Test::FWaitUntilSessionConnectingCompleteCommand::Update()
{
	const EInworldConnectionState ConnectionState = Session->GetConnectionState();
	return ConnectionState != EInworldConnectionState::Idle && ConnectionState != EInworldConnectionState::Connecting;
}

bool Inworld::Test::FWaitUntilSessionDisconnectingCompleteCommand::Update()
{
	const EInworldConnectionState ConnectionState = Session->GetConnectionState();
	return ConnectionState != EInworldConnectionState::Connected;
}

bool Inworld::Test::FTestEqualConnectionStateCommand::Update()
{
	static const UEnum* TypeEnum = StaticEnum<EInworldConnectionState>();
	const FName ExpectedTypeName = TypeEnum->GetNameByValue((int64)ConnectionState);
	const FName CurrentTypeName = TypeEnum->GetNameByValue((int64)Session->GetConnectionState());
	CheckEqual(TEXT("Connection State"), *CurrentTypeName.ToString(), *ExpectedTypeName.ToString());
	return true;
}

bool Inworld::Test::FWaitUntilSessionLoadedCommand::Update()
{
	return Session->IsLoaded();
}

Inworld::Test::FScopedSessionScene::FScopedSessionScene(UInworldSession* InSession, const FInworldScene& InScene, const FString& InWorkspace, const FInworldAuth& InRuntimeAuth)
	: Session(InSession)
{
	StartSessionByScene(Session, InScene, InWorkspace, InRuntimeAuth);
	WaitUntilSessionLoadedWithTimeout(Session, 30.f);
	TestEqualConnectionState(Session, EInworldConnectionState::Connected);
}

Inworld::Test::FScopedSessionScene::~FScopedSessionScene()
{
	StopSession(Session);
	WaitUntilSessionDisconnectingCompleteWithTimeout(Session);
	TestEqualConnectionState(Session, EInworldConnectionState::Idle);
}

Inworld::Test::FScopedSpeechProcessor::FScopedSpeechProcessor(UInworldSession* InSession, EInworldPlayerSpeechMode InPlayerSpeechMode, FInworldPlayerSpeechOptions InPlayerSpeechOptions)
	: Session(InSession)
{
	InitSpeechProcessor(Session, InPlayerSpeechMode, InPlayerSpeechOptions);
}
Inworld::Test::FScopedSpeechProcessor::~FScopedSpeechProcessor()
{
	DestroySpeechProcessor(Session);
}
