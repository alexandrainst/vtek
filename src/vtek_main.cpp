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
};

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

	// 4) allocators for basic Vulkan types
	// TODO: This would be a good place to initialize the allocators.

	return true;
}

void vtek::terminate()
{
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
