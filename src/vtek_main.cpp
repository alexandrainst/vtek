#include "vtek_main.hpp"

#include "impl/vtek_glfw_backend.hpp"
// TODO: No longer use sAllocator ?
//#include "impl/vtek_host_allocator.hpp"
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
static Context* spContext = nullptr;



/* useful stuff */
bool vtek::is_glsl_shader_loading_enabled()
{
	return spContext->useLoadShadersFromGLSL;
}



/* interface */
bool vtek::initialize(const vtek::InitInfo* info)
{
	// 1) static app context
	spContext = new Context();

	// 2) logging
	vtek::initialize_logging(info);

	// 3) optional graphics context for cross-platform window creation (GLFW)
	if (info->useGLFW)
	{
		if (!vtek::glfw_backend_initialize())
		{
			vtek_log_fatal("Failed to initialize GLFW window backend!");
			delete spContext;
			spContext = nullptr;
			return false;
		}

		spContext->useGLFW = true;
	}

	// 4) fileio
	if (!initialize_fileio())
	{
		vtek_log_fatal("Failed to initialize fileio module!");
		delete spContext;
		spContext = nullptr;
		return false;
	}

	// 5) optional shader loading from GLSL source code (glslang)
	if (info->loadShadersFromGLSL)
	{
		if (!vtek::initialize_glsl_shader_loading())
		{
			vtek_log_fatal("Failed to initialize GLSL shader loading backend!");
			delete spContext;
			spContext = nullptr;
			return false;
		}

		spContext->useLoadShadersFromGLSL = true;
	}

	// 6) allocators for basic Vulkan types
	// TODO: This would be a good place to initialize the allocators.

	return true;
}

void vtek::terminate()
{
	if (spContext->useLoadShadersFromGLSL)
	{
		vtek::terminate_glsl_shader_loading();
	}

	vtek::terminate_fileio();

	if (spContext->useGLFW)
	{
		vtek::glfw_backend_terminate();
	}

	// TODO: No longer use sAllocator ?
	//vtek::host_allocator_check_all_freed(); // TODO: This is horrible!

	vtek::terminate_logging();

	delete spContext;
	spContext = nullptr;
}

bool vtek::vtek_context_get_glfw_enabled()
{
	return spContext->useGLFW;
}
