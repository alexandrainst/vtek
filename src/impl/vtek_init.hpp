//
// Provide initialization routines for internal vtek use.
// The functions provided here are implemented by their respective
// source files, e.g.:
// vtek_init_fileio -> vtek_fileio.cpp
//

#pragma once

#include "vtek_types.hpp"


namespace vtek
{
	/* logging */
	// TODO: Probably initialize the logger here?

	/* fileio module */
	bool initialize_fileio();
	void terminate_fileio();

	/* GLSL shader loader */
	bool is_glsl_shader_loading_enabled();
	bool initialize_glsl_shader_loading();
	void terminate_glsl_shader_loading();
	void build_glslang_resource_limits(const PhysicalDevice* physicalDevice);

	/* allocators for basic Vulkan types */
	// TODO: We could place initialization functions here and make everyting better.

}
