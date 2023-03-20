#include "vtek_logging.h"

#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h> // TODO: In source file?


/* helper functions*/
static auto convert_log_level(vtek::LogLevel level)
{
	switch (level)
	{
	case vtek::LogLevel::trace: return spdlog::level::trace;
	case vtek::LogLevel::debug: return spdlog::level::debug;
	case vtek::LogLevel::info:  return spdlog::level::info;
	case vtek::LogLevel::warn:  return spdlog::level::warn;
	case vtek::LogLevel::error: return spdlog::level::err;
	case vtek::LogLevel::fatal: return spdlog::level::critical;
	default:
		return spdlog::level::trace;
	}
}


/* interface */
void vtek::initialize_logging(const vtek::InitInfo* info)
{
	if (info->disableLogging)
	{
		vtek::disable_logging();
		return;
	}

	spdlog::set_pattern("%^[%T] %n(%l): %v%$");
	auto minLevel = convert_log_level(info->minimumLogLevel);

	if (info->logToFile)
	{
		auto daily_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(
			"logfile", 23, 59);

		vtek::LogContainer::sVtekLogger = std::make_shared<spdlog::logger>(
			"vtek", daily_sink);
		vtek::LogContainer::sVtekLogger->set_level(minLevel);

		vtek::LogContainer::sClientLogger = std::make_shared<spdlog::logger>(
			info->applicationTitle, daily_sink);
		vtek::LogContainer::sClientLogger->set_level(minLevel);
	}
	else
	{
		vtek::LogContainer::sVtekLogger = spdlog::stdout_color_mt("vtek");
		vtek::LogContainer::sVtekLogger->set_level(minLevel);

		vtek::LogContainer::sClientLogger = spdlog::stdout_color_mt(
			info->applicationTitle);
		vtek::LogContainer::sClientLogger->set_level(minLevel);
	}

	vtek_log_trace("...initialized logger!");
}

void vtek::terminate_logging()
{
	// TODO: Log a message saying that log is terminating
	vtek_log_trace("Terminating logger...");

	vtek::LogContainer::sVtekLogger->flush();
	vtek::LogContainer::sClientLogger->flush();

	vtek::LogContainer::sVtekLogger = nullptr;
	vtek::LogContainer::sClientLogger = nullptr;

	spdlog::shutdown();
}

void vtek::disable_logging()
{
	if(vtek::LogContainer::sVtekLogger == nullptr) {
		vtek::LogContainer::sVtekLogger = spdlog::stdout_color_mt("null-corelog");
	}
	if(vtek::LogContainer::sClientLogger == nullptr) {
		vtek::LogContainer::sClientLogger = spdlog::stdout_color_mt("null-gamelog");
	}

	vtek::LogContainer::sVtekLogger->set_level(spdlog::level::off);
	vtek::LogContainer::sClientLogger->set_level(spdlog::level::off);
}

void vtek::flush_vtek_logger()
{
	vtek::LogContainer::sVtekLogger->flush();
}

void vtek::flush_client_logger()
{
	vtek::LogContainer::sClientLogger->flush();
}
