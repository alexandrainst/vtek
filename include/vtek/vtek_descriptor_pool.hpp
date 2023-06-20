#pragma once

#include <vector>

#include "vtek_vulkan.pch"
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

	struct DescriptorPoolType
	{
		DescriptorType type;
		uint32_t count {0};
	};

	struct DescriptorPoolInfo
	{
		bool allowIndividualFree {false};
		bool allowUpdateAfterBind {false};

		std::vector<DescriptorType> descriptorTypes;

		// NOTE: From VV:
		// static constexpr uint32_t kMaxTypes = 4;
		// uint32_t numTypes {0};
		// DescriptorPoolType types[kMaxTypes];

		// NOTE: And then initializing in context:
		// info.numTypes = 1;
		// info.types[0].type = DescriptorType::uniform_buffer;
		// info.types[0].count = vtek::swapchain_get_num_frames_in_flight();
	};


	DescriptorPool* descriptor_pool_create(
		DescriptorPoolInfo* info, Device* device);
	void descriptor_pool_destroy(DescriptorPool* pool, Device* device);
}
