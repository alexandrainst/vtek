//
// Logging configuation for vtek.
//
// There are 6 different logging levels, all of which carry special meaning:
// <trace>
// --> May be inserted arbitrarily, and _really_ pollutes the logging output.
// --> Common usage is at the beginning of a function, to ease debugging.
// <debug>
// --> May be used for application state which is not erroneous but may be
// --> optimized, e.g. if an allocator needs to resize itself often.
// <info>
// --> Generalized logging output, to feed information about the platform,
// --> system, hardware configuration, if a service starts or stops, etc.
// --> This format is unrelated to the other logging levels and _should_not_
// --> be used for reporting errors.
// <warn>
// --> Indicates an unexpected program state which should not be.
// --> Execution will likely continue, but if at a later point the
// --> application crashes this log message will be useful.
// <error>
// --> Indicates a severe error in program flow or data state.
// --> No action is forced upon the runtime, but the application may crash.
// <fatal>
// --> Indicates an error which is unrecoverable.
// --> A log message is quickly sent, and then execution is terminated.

#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <spdlog/spdlog.h>

#include "api.hpp"


/*
 * NOTE:
 * For special purposes, such as unit tests, logging can be disabled entirely
 * by defining the following macro: `VTEK_DISABLE_LOGGING'.
 *
 * It is *strongly* discouraged to do this for other use cases!
 */
namespace vtek
{
	void flush_vtek_logger();
	void flush_client_logger();

	class LogContainer
	{
	public:
		static inline std::shared_ptr<spdlog::logger> sVtekLogger = nullptr;
		/*YGG_API*/ static inline std::shared_ptr<spdlog::logger> sClientLogger = nullptr;
	};
}


#if !defined(VTEK_DISABLE_LOGGING)

/*
 * Core logging functions.
 * These are for *internal* usage inside the engine.
 */
template<typename... Args>
inline void vtek_log_trace(const char* message, const Args &... args)
{
	vtek::LogContainer::sVtekLogger->trace(
		SPDLOG_FMT_RUNTIME(message), args...);
}

template<typename... Args>
inline void vtek_log_debug(const char* message, const Args &... args)
{
	vtek::LogContainer::sVtekLogger->debug(
		SPDLOG_FMT_RUNTIME(message), args...);
}

template<typename... Args>
inline void vtek_log_info(const char* message, const Args &... args)
{
	vtek::LogContainer::sVtekLogger->info(
		SPDLOG_FMT_RUNTIME(message), args...);
}

template<typename... Args>
inline void vtek_log_warn(const char* message, const Args &... args)
{
	vtek::LogContainer::sVtekLogger->warn(
		SPDLOG_FMT_RUNTIME(message), args...);
}

template<typename... Args>
inline void vtek_log_error(const char* message, const Args &... args)
{
	vtek::LogContainer::sVtekLogger->error(
		SPDLOG_FMT_RUNTIME(message), args...);
}

template<typename... Args>
inline void vtek_log_fatal(const char* message, const Args &... args)
{
	vtek::LogContainer::sVtekLogger->critical(
		SPDLOG_FMT_RUNTIME(message), args...);
}



/*
 * Client logging functions.
 * These are provided by DLL-export annotations and should be used by client apps.
 */
template<typename... Args>
VTEK_API inline void log_trace(const char* message, const Args &... args)
{
	//#if defined(YGG_DEBUG) && !defined(YGG_DISABLE_LOGGING)
	vtek::LogContainer::sClientLogger->trace(
		SPDLOG_FMT_RUNTIME(message), args...);
	//#endif
}

template<typename... Args>
VTEK_API inline void log_debug(const char* message, const Args &... args)
{
	vtek::LogContainer::sClientLogger->debug(
		SPDLOG_FMT_RUNTIME(message), args...);
}

template<typename... Args>
VTEK_API inline void log_info(const char* message, const Args &... args)
{
	vtek::LogContainer::sClientLogger->info(
		SPDLOG_FMT_RUNTIME(message), args...);
}

template<typename... Args>
VTEK_API inline void log_warn(const char* message, const Args &... args)
{
	vtek::LogContainer::sClientLogger->warn(
		SPDLOG_FMT_RUNTIME(message), args...);
}

template<typename... Args>
VTEK_API inline void log_error(const char* message, const Args &... args)
{
	vtek::LogContainer::sClientLogger->error(
		SPDLOG_FMT_RUNTIME(message), args...);
}

template<typename... Args>
VTEK_API inline void log_fatal(const char* message, const Args &... args)
{
	vtek::LogContainer::sClientLogger->critical(
		SPDLOG_FMT_RUNTIME(message), args...);
}


#else

template<typename... Args>
inline void vtek_log_trace(const char* message, const Args &... args) {}

template<typename... Args>
inline void vtek_log_debug(const char* message, const Args &... args) {}

template<typename... Args>
inline void vtek_log_info(const char* message, const Args &... args) {}

template<typename... Args>
inline void vtek_log_warn(const char* message, const Args &... args) {}

template<typename... Args>
inline void vtek_log_error(const char* message, const Args &... args) {}

template<typename... Args>
inline void vtek_log_fatal(const char* message, const Args &... args) {}

template<typename... Args>
VTEK_API inline void log_trace(const char* message, const Args &... args) {}

template<typename... Args>
VTEK_API inline void log_debug(const char* message, const Args &... args) {}

template<typename... Args>
VTEK_API inline void log_info(const char* message, const Args &... args) {}

template<typename... Args>
VTEK_API inline void log_warn(const char* message, const Args &... args) {}

template<typename... Args>
VTEK_API inline void log_error(const char* message, const Args &... args) {}

template<typename... Args>
VTEK_API inline void log_fatal(const char* message, const Args &... args) {}

#endif
