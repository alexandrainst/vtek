#include "vtek_main.hpp"

#include "impl/vtek_glfw_backend.hpp"
#include "impl/vtek_host_allocator.hpp"
#include "impl/vtek_init.hpp"
#include "vtek_logging.hpp"

#include <iostream>


/* vtek context */
struct Context
{
	bool useGLFW {false};
	bool useLoadShadersFromGLSL {false};
};

// TODO: Would dynamic memory allocation be better?
static Context sContext = {};



/* interface */
bool vtek::initialize(const vtek::InitInfo* info)
{
	// 1) logging
	vtek::initialize_logging(info);

	// 2) optional graphics context for cross-platform window creation (GLFW)
	if (info->useGLFW)
	{
		if (!vtek::glfw_backend_initialize())
		{
			vtek_log_fatal("Failed to initialize GLFW window backend!");
			return false;
		}

		sContext.useGLFW = true;
	}

	// 3) fileio
	if (!initialize_fileio())
	{
		vtek_log_fatal("Failed to initialize fileio module!");
		return false;
	}

	// 4) optional shader loading from GLSL source code (glslang)
	if (info->loadShadersFromGLSL)
	{
		if (!vtek::initialize_glsl_shader_loading())
		{
			vtek_log_fatal("Failed to initialize GLSL shader loading backend!");
			return false;
		}

		sContext.useLoadShadersFromGLSL = true;
	}

	// 5) allocators for basic Vulkan types
	// TODO: This would be a good place to initialize the allocators.

	return true;
}

void vtek::terminate()
{
	if (sContext.useLoadShadersFromGLSL)
	{
		vtek::terminate_glsl_shader_loading();
	}

	vtek::terminate_fileio();

	if (sContext.useGLFW)
	{
		vtek::glfw_backend_terminate();
	}

	vtek::host_allocator_check_all_freed(); // TODO: This is horrible!

	vtek::terminate_logging();

	sContext = {};
}

bool vtek::vtek_context_get_glfw_enabled()
{
	return sContext.useGLFW;
}
