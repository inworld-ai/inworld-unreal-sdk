/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include <string>
#include <memory>

namespace Inworld
{
	extern std::string g_SessionId;

	void LogSetSessionId(const std::string Id);
	void LogClearSessionId();

#ifdef INWORLD_LOG_CUSTOM
	struct Logger
	{
        virtual ~Logger() = default;
		virtual void Log(const std::string& message) = 0;
		virtual void LogWarning(const std::string& message) = 0;
		virtual void LogError(const std::string& message) = 0;
	};
	extern std::unique_ptr<Logger> g_Logger;

	template<typename T>
	void LogSetLogger() { g_Logger = std::make_unique<T>(); }
	void LogClearLogger();
#endif

	template<typename... Args>
	std::string VFormat(std::string fmt, Args &&... args)
	{
#ifdef INWORLD_LOG_FORMAT_SPD

#ifdef _WIN32
		namespace format = std;
#else
		namespace format = fmt;
#endif

		for (int32_t i = 0; i < fmt.size(); i++)
		{
			if (fmt[i] == '%')
			{
				fmt[i] = '{';
				fmt[i + 1] = '}';
			}
		}
		return format::vformat(fmt, format::make_format_args(args...));
#else
		size_t size = std::snprintf(nullptr, 0, fmt.c_str(), std::forward<Args>(args)...) + 1;
		std::unique_ptr<char[]> buf(new char[size]);
		std::snprintf(buf.get(), size, fmt.c_str(), std::forward<Args>(args)...);
		return std::string(buf.get(), buf.get() + size - 1);
#endif
	}
	
	void Log(const std::string& message);

	template<typename... Args>
	void Log(std::string fmt, Args &&... args)
	{
#ifdef INWORLD_LOG
		Log(VFormat(fmt, args...));
#endif
	}

	void LogWarning(const std::string& message);

	template<typename... Args>
	void LogWarning(std::string fmt, Args &&... args)
	{
#ifdef INWORLD_LOG
		LogWarning(VFormat(fmt, args...));
#endif
	}
	
	void LogError(const std::string& message);

	template<typename... Args>
	void LogError(std::string fmt, Args &&... args)
	{
#ifdef INWORLD_LOG
		LogError(VFormat(fmt, args...));
#endif
	}

	#define ARG_STR(str) str.c_str()
	#define ARG_CHAR(str) str

}
