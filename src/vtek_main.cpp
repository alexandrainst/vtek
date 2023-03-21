#include "impl/vtek_glfw_backend.h"
#include "impl/vtek_host_allocator.h"
#include "vtek_main.h"
#include "vtek_logging.h"

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
	std::cout << "1\n";
	vtek::initialize_logging(info);
	std::cout << "2\n";

	// if (!vtek::host_allocator_initialize())
	// {
	// 	vtek_log_fatal("Failed to initialize vtek host allocator!");
	// 	return false;
	// }

	// if (info->useGLFW)
	// {
	// 	if (!vtek::glfw_backend_initialize())
	// 	{
	// 		vtek_log_error("Failed to initialize GLFW window backend!");
	// 		return false;
	// 	}

	// 	sContext.useGLFW = true;
	// }

	return true;
}

void vtek::terminate()
{
	if (sContext.useGLFW)
	{
		vtek::glfw_backend_terminate();
	}

	vtek::host_allocator_destroy();
	vtek::terminate_logging();

	sContext = {};
}

bool vtek::vtek_context_get_glfw_enabled()
{
	return sContext.useGLFW;
}
