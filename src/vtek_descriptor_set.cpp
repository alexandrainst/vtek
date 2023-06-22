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

void vtek::descriptor_set_apply_updates(
	vtek::DescriptorSet* set, vtek::Device* device)
{
	VkDevice dev = vtek::device_get_handle(device);

	vkUpdateDescriptorSets(dev, size, data, 0, nullptr); // TODO: ?
}



/* update descriptors */
bool descriptor_set_add_sampler();

bool descriptor_set_add_combined_image_sampler();

bool descriptor_set_add_sampled_image();

bool descriptor_set_add_uniform_texel_buffer();

bool descriptor_set_add_storage_texel_buffer();

bool vtek::descriptor_set_add_uniform_buffer(
	vtek::DescriptorSet* set, uint32_t binding,
	vtek::Buffer* buffer, const BufferRegion* region)
{
	VkWriteDescriptorSet writeSet{};
	writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeSet.pNext = nullptr;
	writeSet.dstSet = set->vulkanHandle;
	writeSet.dstBinding = binding; // TODO: Is this binding valid?

	// If the descriptor binding identified by dstSet and dstBinding has a
	// descriptor type of INLINE_UNIFORM_BLOCK, then dstArrayElement
	// specifies the starting byte offset withing the binding.
	writeSet.dstArrayElement = 0;
	writeSet.descriptorType =; // TODO: VkDescriptorType
	writeSet.descriptorCount =
}

bool descriptor_set_add_storage_buffer();

bool descriptor_set_add_uniform_buffer_dynamic();

bool descriptor_set_add_storage_buffer_dynamic();

bool descriptor_set_add_input_attachment();

bool descriptor_set_add_inline_uniform_block();



