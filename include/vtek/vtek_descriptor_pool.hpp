#pragma once

#include <vulkan/vulkan.h>
#include <optional>
#include <vector>

#include "vtek_vulkan_handles.hpp"
#include "vtek_descriptor_type.hpp"


namespace vtek
{
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
