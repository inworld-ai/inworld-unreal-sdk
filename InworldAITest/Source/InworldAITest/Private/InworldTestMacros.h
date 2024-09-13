/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "InworldAITestModule.h"

#define DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_TIMEOUT_CLASS(CommandName, Constructor, Destructor, Body) \
	class F##CommandName##CommandTimeout : public F##CommandName##Command \
	{ \
	public: \
		Constructor \
		Destructor \
		Body \
	} \

#define INWORLD_TEST_LATENT_AUTOMATION_COMMAND_TIMEOUT_CONSTRUCTOR(CommandName) \
	public: \
	F##CommandName##CommandTimeout(float Timeout) \
		: F##CommandName##Command() \
		, _Timeout(Timeout) \
		{} \

#define INWORLD_TEST_LATENT_AUTOMATION_COMMAND_TIMEOUT_CONSTRUCTOR_ONE_PARAM(CommandName, ParamType, ParamName) \
	public: \
	F##CommandName##CommandTimeout(ParamType ParamName, float Timeout) \
		: F##CommandName##Command(ParamName) \
		, _Timeout(Timeout) \
		{} \

#define INWORLD_TEST_LATENT_AUTOMATION_COMMAND_TIMEOUT_CONSTRUCTOR_TWO_PARAM(CommandName, ParamType0, ParamName0, ParamType1, ParamName1) \
	public: \
	F##CommandName##CommandTimeout(ParamType0 ParamName0, ParamType1 ParamName1, float Timeout) \
		: F##CommandName##Command(ParamName0, ParamName1) \
		, _Timeout(Timeout) \
		{} \

#define INWORLD_TEST_LATENT_AUTOMATION_COMMAND_TIMEOUT_CONSTRUCTOR_THREE_PARAM(CommandName, ParamType0, ParamName0, ParamType1, ParamName1, ParamType2, ParamName2) \
	public: \
	F##CommandName##CommandTimeout(ParamType0 ParamName0, ParamType1 ParamName1, ParamType2 ParamName2, float Timeout) \
		: F##CommandName##Command(ParamName0, ParamName1, ParamName2) \
		, _Timeout(Timeout) \
		{} \

#define INWORLD_TEST_LATENT_AUTOMATION_COMMAND_TIMEOUT_CONSTRUCTOR_FOUR_PARAM(CommandName, ParamType0, ParamName0, ParamType1, ParamName1, ParamType2, ParamName2, ParamType3, ParamName3) \
	public: \
	F##CommandName##CommandTimeout(ParamType0 ParamName0, ParamType1 ParamName1,ParamType2 ParamName2, ParamType3 ParamName3, float Timeout) \
		: F##CommandName##Command(ParamName0, ParamName1, ParamName2, ParamName3) \
		, _Timeout(Timeout) \
		{} \

#define INWORLD_TEST_LATENT_AUTOMATION_COMMAND_TIMEOUT_CONSTRUCTOR_FIVE_PARAM(CommandName, ParamType0, ParamName0, ParamType1, ParamName1, ParamType2, ParamName2, ParamType3, ParamName3, ParamType4, ParamName4) \
	public: \
	F##CommandName##CommandTimeout(ParamType0 ParamName0, ParamType1 ParamName1, ParamType2 ParamName2, ParamType3 ParamName3, ParamType4 ParamName4, float Timeout) \
		: F##CommandName##Command(ParamName0, ParamName1, ParamName2, ParamName3, ParamName4) \
		, _Timeout(Timeout) \
		{} \

#define INWORLD_TEST_LATENT_AUTOMATION_COMMAND_TIMEOUT_DESTRUCTOR(CommandName) \
	public: \
	virtual ~F##CommandName##CommandTimeout() \
	{} \

#define INWORLD_TEST_LATENT_AUTOMATION_COMMAND_TIMEOUT_BODY(CommandName) \
	public: \
	virtual bool Update() override \
		{ \
			if(F##CommandName##Command::Update()) return true; \
			if (FPlatformTime::Seconds() - StartTime >= _Timeout) \
			{\
				return true;\
			}\
			return false; \
		} \
	private: \
		float _Timeout; \

#define DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND(CommandName) \
	DEFINE_LATENT_AUTOMATION_COMMAND(F##CommandName##Command); \
	DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_TIMEOUT_CLASS(CommandName, \
		INWORLD_TEST_LATENT_AUTOMATION_COMMAND_TIMEOUT_CONSTRUCTOR(CommandName), \
		INWORLD_TEST_LATENT_AUTOMATION_COMMAND_TIMEOUT_DESTRUCTOR(CommandName), \
		INWORLD_TEST_LATENT_AUTOMATION_COMMAND_TIMEOUT_BODY(CommandName) \
	); \
	void CommandName() { ADD_LATENT_AUTOMATION_COMMAND(F##CommandName##Command(ParamName)); } \
	void CommandName##WithTimeout(float Timeout) { ADD_LATENT_AUTOMATION_COMMAND(F##CommandName##CommandTimeout(Timeout)); } \

#define DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(CommandName, ParamType, ParamName) \
	DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(F##CommandName##Command, ParamType, ParamName); \
	DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_TIMEOUT_CLASS(CommandName, \
		INWORLD_TEST_LATENT_AUTOMATION_COMMAND_TIMEOUT_CONSTRUCTOR_ONE_PARAM(CommandName, ParamType, ParamName), \
		INWORLD_TEST_LATENT_AUTOMATION_COMMAND_TIMEOUT_DESTRUCTOR(CommandName), \
		INWORLD_TEST_LATENT_AUTOMATION_COMMAND_TIMEOUT_BODY(CommandName) \
	); \
	void CommandName(ParamType ParamName) { ADD_LATENT_AUTOMATION_COMMAND(F##CommandName##Command(ParamName)); } \
	void CommandName##WithTimeout(ParamType ParamName, float Timeout) { ADD_LATENT_AUTOMATION_COMMAND(F##CommandName##CommandTimeout(ParamName, Timeout)); } \

#define DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(CommandName, ParamType0, ParamName0, ParamType1, ParamName1) \
	DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(F##CommandName##Command, ParamType0, ParamName0, ParamType1, ParamName1); \
	DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_TIMEOUT_CLASS(CommandName, \
		INWORLD_TEST_LATENT_AUTOMATION_COMMAND_TIMEOUT_CONSTRUCTOR_TWO_PARAM(CommandName, ParamType0, ParamName0, ParamType1, ParamName1), \
		INWORLD_TEST_LATENT_AUTOMATION_COMMAND_TIMEOUT_DESTRUCTOR(CommandName), \
		INWORLD_TEST_LATENT_AUTOMATION_COMMAND_TIMEOUT_BODY(CommandName) \
	); \
	void CommandName(ParamType0 ParamName0, ParamType1 ParamName1) { ADD_LATENT_AUTOMATION_COMMAND(F##CommandName##Command(ParamName0, ParamName1)); } \
	void CommandName##WithTimeout(ParamType0 ParamName0, ParamType1 ParamName1, float Timeout) { ADD_LATENT_AUTOMATION_COMMAND(F##CommandName##CommandTimeout(ParamName0, ParamName1, Timeout)); } \

#define DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(CommandName, ParamType0, ParamName0, ParamType1, ParamName1, ParamType2, ParamName2) \
	DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(F##CommandName##Command, ParamType0, ParamName0, ParamType1, ParamName1, ParamType2, ParamName2); \
	DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_TIMEOUT_CLASS(CommandName, \
		INWORLD_TEST_LATENT_AUTOMATION_COMMAND_TIMEOUT_CONSTRUCTOR_THREE_PARAM(CommandName, ParamType0, ParamName0, ParamType1, ParamName1, ParamType2, ParamName2), \
		INWORLD_TEST_LATENT_AUTOMATION_COMMAND_TIMEOUT_DESTRUCTOR(CommandName), \
		INWORLD_TEST_LATENT_AUTOMATION_COMMAND_TIMEOUT_BODY(CommandName) \
	); \
	void CommandName(ParamType0 ParamName0, ParamType1 ParamName1, ParamType2 ParamName2) { ADD_LATENT_AUTOMATION_COMMAND(F##CommandName##Command(ParamName0, ParamName1, ParamName2)); } \
	void CommandName##WithTimeout(ParamType0 ParamName0, ParamType1 ParamName1, ParamType2 ParamName2, float Timeout) { ADD_LATENT_AUTOMATION_COMMAND(F##CommandName##CommandTimeout(ParamName0, ParamName1, ParamName2, Timeout)); } \

#define DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_FOUR_PARAMETER(CommandName, ParamType0, ParamName0, ParamType1, ParamName1, ParamType2, ParamName2, ParamType3, ParamName3) \
	DEFINE_LATENT_AUTOMATION_COMMAND_FOUR_PARAMETER(F##CommandName##Command, ParamType0, ParamName0, ParamType1, ParamName1, ParamType2, ParamName2, ParamType3, ParamName3); \
	DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_TIMEOUT_CLASS(CommandName, \
		INWORLD_TEST_LATENT_AUTOMATION_COMMAND_TIMEOUT_CONSTRUCTOR_FOUR_PARAM(CommandName, ParamType0, ParamName0, ParamType1, ParamName1, ParamType2, ParamName2, ParamType3, ParamName3), \
		INWORLD_TEST_LATENT_AUTOMATION_COMMAND_TIMEOUT_DESTRUCTOR(CommandName), \
		INWORLD_TEST_LATENT_AUTOMATION_COMMAND_TIMEOUT_BODY(CommandName) \
	); \
	void CommandName(ParamType0 ParamName0, ParamType1 ParamName1, ParamType2 ParamName2, ParamType3 ParamName3) { ADD_LATENT_AUTOMATION_COMMAND(F##CommandName##Command(ParamName0, ParamName1, ParamName2, ParamName3)); } \
	void CommandName##WithTimeout(ParamType0 ParamName0, ParamType1 ParamName1, ParamType2 ParamName2, ParamType3 ParamName3, float Timeout) { ADD_LATENT_AUTOMATION_COMMAND(F##CommandName##CommandTimeout(ParamName0, ParamName1, ParamName2, ParamName3, Timeout)); } \

#define DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_FIVE_PARAMETER(CommandName, ParamType0, ParamName0, ParamType1, ParamName1, ParamType2, ParamName2, ParamType3, ParamName3, ParamType4, ParamName4) \
	DEFINE_LATENT_AUTOMATION_COMMAND_FIVE_PARAMETER(F##CommandName##Command, ParamType0, ParamName0, ParamType1, ParamName1, ParamType2, ParamName2, ParamType3, ParamName3, ParamType4, ParamName4); \
	DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_TIMEOUT_CLASS(CommandName, \
		INWORLD_TEST_LATENT_AUTOMATION_COMMAND_TIMEOUT_CONSTRUCTOR_FIVE_PARAM(CommandName, ParamType0, ParamName0, ParamType1, ParamName1, ParamType2, ParamName2, ParamType3, ParamName3, ParamType4, ParamName4), \
		INWORLD_TEST_LATENT_AUTOMATION_COMMAND_TIMEOUT_DESTRUCTOR(CommandName), \
		INWORLD_TEST_LATENT_AUTOMATION_COMMAND_TIMEOUT_BODY(CommandName) \
	); \
	void CommandName(ParamType0 ParamName0, ParamType1 ParamName1, ParamType2 ParamName2, ParamType3 ParamName3, ParamType4 ParamName4) { ADD_LATENT_AUTOMATION_COMMAND(F##CommandName##Command(ParamName0, ParamName1, ParamName2, ParamName3, ParamName4)); } \
	void CommandName##WithTimeout(ParamType0 ParamName0, ParamType1 ParamName1, ParamType2 ParamName2, ParamType3 ParamName3, ParamType4 ParamName4, float Timeout) { ADD_LATENT_AUTOMATION_COMMAND(F##CommandName##CommandTimeout(ParamName0, ParamName1, ParamName2, ParamName3, ParamName4, Timeout)); } \
