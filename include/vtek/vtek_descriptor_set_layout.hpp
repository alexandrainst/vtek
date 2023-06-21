#pragma once

#include <vulkan/vulkan.h>

#include "vtek_vulkan_handles.hpp"


namespace vtek
{
	struct DescriptorSetLayoutInfo
	{
	};


	DescriptorSetLayout* descriptor_set_layout_create(
		const DescriptorSetLayoutInfo* info);
	void descriptor_set_layout_destroy(DescriptorSetLayout* layout);
}
