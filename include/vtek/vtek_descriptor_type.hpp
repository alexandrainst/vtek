#pragma once

#include <vulkan/vulkan.h>
#include <optional>

#include "vtek_object_handles.hpp"


namespace vtek
{
	enum class DescriptorType
	{
		sampler,
		combined_image_sampler,
		sampled_image,
		storage_image,
		uniform_texel_buffer,
		storage_texel_buffer,
		uniform_buffer,
		storage_buffer,
		uniform_buffer_dynamic,
		storage_buffer_dynamic,
		input_attachment,

		// Provided by VK_VERSION_1_3
		inline_uniform_block,

		// Provided by VK_KHR_acceleration_structure
		acceleration_structure
	};

	std::optional<VkDescriptorType> get_descriptor_type(
		DescriptorType type, Device* device);
}
