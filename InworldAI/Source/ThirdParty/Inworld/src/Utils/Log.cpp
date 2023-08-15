/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "Log.h"

#if INWORLD_LOG_CUSTOM
namespace Inworld
{
	std::unique_ptr<Logger> g_Logger = nullptr;
	void LogClearLogger() { g_Logger = nullptr; }
}
#elif UNITY_NDK
#elif ANDROID
	#include <android/log.h>
#else
	#include "spdlog/spdlog.h"
#endif

namespace Inworld
{
	std::string g_SessionId = "Unknown";

	void LogSetSessionId(const std::string Id)
	{
		g_SessionId = Id;
	}

	void LogClearSessionId()
	{
		g_SessionId = "Unknown";
	}

}

void Inworld::Log(const std::string& message)
{
#ifdef INWORLD_LOG
	#ifdef INWORLD_LOG_CUSTOM
		if (g_Logger) g_Logger->Log(message);
	#elif UNITY_NDK
		std::cout << message << std::endl;
	#elif ANDROID
		__android_log_print(ANDROID_LOG_INFO, "InworldNDK", "%s", message.c_str());
	#else
		spdlog::info(message);
	#endif
#endif
}

void Inworld::LogWarning(const std::string& message)
{
#ifdef INWORLD_LOG
	#ifdef INWORLD_LOG_CUSTOM
		if (g_Logger) g_Logger->LogWarning(message);
	#elif UNITY_NDK
		std::cout << message << std::endl;
	#elif ANDROID
		__android_log_print(ANDROID_LOG_WARN, "InworldNDK", "%s", message.c_str());
	#else
		spdlog::warn(message);
	#endif
#endif
}

void Inworld::LogError(const std::string& message)
{
#ifdef INWORLD_LOG
	const std::string error = VFormat("%s (SessionId: %s)", ARG_STR(message), ARG_STR(g_SessionId));
	#ifdef INWORLD_LOG_CUSTOM
	if (g_Logger) g_Logger->LogError(error);
	#elif UNITY_NDK
		std::cout << error << std::endl;
	#elif ANDROID
		__android_log_print(ANDROID_LOG_ERROR, "InworldNDK", "%s", error.c_str());
	#else
		spdlog::error(error);
	#endif
#endif
}

