#pragma once

#include "vtek_fileio.hpp"
#include "vtek_shaders.hpp"
#include "vtek_object_handles.hpp"
#include "vtek_vulkan_version.hpp"

#include <cstdint>
#include <vector>


namespace vtek
{
	bool glsl_utils_initialize();
	void glsl_utils_terminate();

	void glsl_utils_build_resource_limits(
		const PhysicalDevice* physicalDevice);

	std::vector<uint32_t> glsl_utils_load_shader(
		vtek::Directory* shaderdir, const char* filename,
		vtek::ShaderStageGraphics stage, vtek::VulkanVersion apiVersion,
		VkDevice dev);
}
