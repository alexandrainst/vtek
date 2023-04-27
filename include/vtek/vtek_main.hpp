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

		// === window creation ===
		// This must be set to `true` if GLFW is used for window creation.
		// If this is not set, then any effort to call GLFW functions will result in application crash.
		bool useGLFW {false};

		// === logging ===
		bool disableLogging {false};
		LogLevel minimumLogLevel {LogLevel::trace};
		bool logToFile {false};
		bool multiThreadedLogging {false}; // TODO: This is not implemented!

		// === glsl shader loading ===
		// Set to `true` to enable loading and parsing Vulkan shaders from GLSL source code.
		// If this is not set, then loading shaders from GLSL source code will always fail.
		bool loadShadersFromGLSL {false};
	};

	bool initialize(const InitInfo* info);
	void terminate();


	// context query
	bool vtek_context_get_glfw_enabled();
}
