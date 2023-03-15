#include "vtek_logging.h"

#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h> // TODO: In source file?


/* helper functions*/
static auto convert_loglevel(vtek::LogLevel level)
{
	switch (level)
	{
	case vtek::LogLevel::trace: return spdlog::level::trace;
	case vtek::LogLevel::debug: return spdlog::level::debug;
	case vtek::LogLevel::info:  return spdlog::level::info;
	case vtek::LogLevel::warn:  return spdlog::level::warn;
	case vtek::LogLevel::error: return spdlog::level::error;
	case vtek::LogLevel::fatal: return spdlog::level::fatal;
	default:
		return spdlog::level::trace;
	}
}


/* interface */
void vtek::initialize_logging(const LoggingCreateInfo* info)
{
	spdlog::set_pattern("%^[%T] %n(%l): %v%$");
	auto minLevel = convert_log_level(info->minimumLogLevel);

	if (info->logToFile)
	{
		auto daily_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(
			"logfile", 23, 59);

		core::LogContainer::sVtekLogger = std::make_shared<spdlog::logger>(
			"vtek", daily_sink);
		core::LogContainer::sVtekLogger->set_level(minLevel);

		core::LogContainer::sClientLogger = std::make_shared<spdlog::logger>(
			info->applicationTitle, daily_sink);
		core::LogContainer::sClientLogger->set_level(minLevel);
	}
	else
	{
		core::LogContainer::sVtekLogger = spdlog::stdout_color_mt("vtek");
		core::LogContainer::sVtekLogger->set_level(minLevel);

		core::LogContainer::sClientLogger = spdlog::stdout_color_mt(
			info->applicationTitle);
		core::LogContainer::sClientLogger->set_level(minLevel);
	}

	log_core_trace("...initialized logger!");
}

void core::terminate_logging()
{
	// TODO: Log a message saying that log is terminating
	log_core_trace("Terminating core logger...");

	core::LogContainer::sVtekLogger->flush();
	core::LogContainer::sClientLogger->flush();

	core::LogContainer::sVtekLogger = nullptr;
	core::LogContainer::sClientLogger = nullptr;

	spdlog::shutdown();
}

void core::disable_logging()
{
	if(core::LogContainer::sVtekLogger == nullptr) {
		core::LogContainer::sVtekLogger = spdlog::stdout_color_mt("null-corelog");
	}
	if(core::LogContainer::sClientLogger == nullptr) {
		core::LogContainer::sClientLogger = spdlog::stdout_color_mt("null-gamelog");
	}

	core::LogContainer::sVtekLogger->set_level(spdlog::level::off);
	core::LogContainer::sClientLogger->set_level(spdlog::level::off);
}

void core::flush_core_logger()
{
	core::LogContainer::sVtekLogger->flush();
}

void core::flush_game_logger()
{
	core::LogContainer::sClientLogger->flush();
}
