#pragma once

#include <vulkan/vulkan.h>
#include <optional>
#include <vector>

#include "vtek_vulkan_handles.hpp"


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
		DescriptorType type, Device device);

	struct DescriptorPoolType
	{
		DescriptorType type;
		uint32_t count {0};
	};

	struct DescriptorPoolInfo
	{
		bool allowIndividualFree {false};
		bool allowUpdateAfterBind {false};

		std::vector<DescriptorPoolType> descriptorTypes;
	};


	DescriptorPool* descriptor_pool_create(
		DescriptorPoolInfo* info, Device* device);
	void descriptor_pool_destroy(DescriptorPool* pool, Device* device);

	VkDescriptorPool descriptor_pool_get_handle(DescriptorPool* pool);
}
