#pragma once

#include "vtek_glm_includes.hpp"


namespace vtek
{
	enum class UniformBufferType
	{
		vec3
	};

	uint64_t get_uniform_buffer_size(UniformBufferType type);

	// TODO: Consider different types of uniforms? (layouts?)

	// TODO: Consider a struct for each uniform type
	// NOTE: Let each struct have a uniform type, and pass that uniform type
	// to functions that update descriptor sets!
}
