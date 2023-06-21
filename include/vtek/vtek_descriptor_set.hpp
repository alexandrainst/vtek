#pragma once

#include "vtek_vulkan_handles.hpp"


namespace vtek
{
	DescriptorSet* descriptor_set_create(
		DescriptorPool* pool, DescriptorSetLayout* layout, Device* device);

	// TODO: Has pool been created with destroy individual flag?
	void descriptor_set_destroy(DescriptorSet* set);
}
