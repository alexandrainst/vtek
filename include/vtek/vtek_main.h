#pragma once


namespace vtek
{
	enum class LogLevel
	{
		trace, debug, info, warn, error, fatal
	};

	struct InitInfo
	{
		// === app info ===
		const char* applicationTitle {"app"}; // TODO: For logging purposes should always be "app" ??

		// === logging ===
		bool disableLogging {false};
		LogLevel minimumLogLevel {LogLevel::trace};
		bool logToFile {false};
		bool multiThreadedLogging {false}; // TODO: This is not implemented!
	};

	bool initialize(const InitInfo* info);
	void terminate();
}
