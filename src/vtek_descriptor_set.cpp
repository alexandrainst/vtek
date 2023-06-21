#include "vtek_vulkan.pch"
#include "vtek_descriptor_set.hpp"

#include "vtek_descriptor_pool.hpp"
#include "vtek_descriptor_set_layout.hpp"
#include "vtek_device.hpp"
#include "vtek_logging.hpp"


/* struct implementation */
struct vtek::DescriptorSet
{
	VkDescriptorSet vulkanHandle {VK_NULL_HANDLE};
};



/* interface */
vtek::DescriptorSet* vtek::descriptor_set_create(
	vtek::DescriptorPool* pool, vtek::DescriptorSetLayout* layout,
	vtek::Device* device)
{
	VkDevice dev = vtek::device_get_handle(device);

	auto set = new vtek::DescriptorSet;

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = vtek::descriptor_pool_get_handle(pool);
	allocInfo.descriptorSetCount = 1;
	VkDescriptorSetLayout layouts[1] = {
		vtek::descriptor_set_layout_get_handle(layout)
	};
	allocInfo.pSetLayouts = layouts;

	VkResult result =
		vkAllocateDescriptorSets(dev, &allocInfo, &set->vulkanHandle);
	if (result != VK_SUCCESS)
	{
		vtek_log_error("Failed to allocate descriptor set from pool!");
		delete set;
		return nullptr;
	}

	return set;
}

void vtek::descriptor_set_destroy(vtek::DescriptorSet* set)
{
	vtek_log_error("vtek::descriptor_set_destroy(): Not implemented!");
}
