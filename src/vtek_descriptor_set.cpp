#include "vtek_vulkan.pch"
#include "vtek_descriptor_set.hpp"

#include "vtek_buffer.hpp"
#include "vtek_descriptor_pool.hpp"
#include "vtek_descriptor_set_layout.hpp"
#include "vtek_device.hpp"
#include "vtek_logging.hpp"

#include <deque>
#include <vector>


/* struct implementation */
struct vtek::DescriptorSet
{
	VkDescriptorSet vulkanHandle {VK_NULL_HANDLE};

	std::deque<VkDescriptorBufferInfo> bufferInfos;
	std::vector<VkWriteDescriptorSet> writeDescriptors;
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

VkDescriptorSet vtek::descriptor_set_get_handle(vtek::DescriptorSet* set)
{
	return set->vulkanHandle;
}

void vtek::descriptor_set_update(
	vtek::DescriptorSet* set, vtek::Device* device)
{
	VkDevice dev = vtek::device_get_handle(device);

	vkUpdateDescriptorSets(
		dev, set->writeDescriptors.size(), set->writeDescriptors.data(),
		0, nullptr); // TODO: ?

	set->writeDescriptors.clear();
	set->bufferInfos.clear();
}



/* update descriptors */
bool vtek::descriptor_set_bind_sampler()
{
	vtek_log_error("vtek::descriptor_set_bind_sampler(): {}",
	               "Not implemented!");
	return false;
}

bool vtek::descriptor_set_bind_combined_image_sampler()
{
	vtek_log_error("vtek::descriptor_set_bind_combined_image_sampler(): {}",
	               "Not implemented!");
	return false;
}

bool vtek::descriptor_set_bind_sampled_image()
{
	vtek_log_error("vtek::descriptor_set_bind_sampled_image(): {}",
	               "Not implemented!");
	return false;
}

bool vtek::descriptor_set_bind_uniform_texel_buffer()
{
	vtek_log_error("vtek::descriptor_set_bind_uniform_texel_buffer(): {}",
	               "Not implemented!");
	return false;
}

bool vtek::descriptor_set_bind_storage_texel_buffer()
{
	vtek_log_error("vtek::descriptor_set_bind_storage_texel_buffer(): {}",
	               "Not implemented!");
	return false;
}

bool vtek::descriptor_set_bind_uniform_buffer(
	vtek::DescriptorSet* set, uint32_t binding,
	vtek::Buffer* buffer, vtek::UniformBufferType type)
{
	VkDescriptorBufferInfo bufferInfo{};
	bufferInfo.buffer = vtek::buffer_get_handle(buffer);
	bufferInfo.offset = 0;
	bufferInfo.range = vtek::get_uniform_buffer_size(type);
	set->bufferInfos.emplace_back(bufferInfo);

	VkWriteDescriptorSet writeSet{};
	writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeSet.pNext = nullptr;
	writeSet.dstSet = set->vulkanHandle;
	writeSet.dstBinding = binding; // TODO: Is this binding valid?

	// If the descriptor binding identified by dstSet and dstBinding has a
	// descriptor type of INLINE_UNIFORM_BLOCK, then dstArrayElement
	// specifies the starting byte offset withing the binding.
	writeSet.dstArrayElement = 0;
	writeSet.descriptorCount = 1; // Number of elements in `pBufferInfo`.
	writeSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writeSet.pImageInfo = nullptr;
	writeSet.pBufferInfo = &(set->bufferInfos.back());
	writeSet.pTexelBufferView = nullptr;

	set->writeDescriptors.emplace_back(writeSet);

	return true;
}

bool vtek::descriptor_set_bind_storage_buffer()
{
	vtek_log_error("vtek::descriptor_set_bind_storage_buffer(): {}",
	               "Not implemented!");
	return false;
}

bool vtek::descriptor_set_bind_uniform_buffer_dynamic()
{
	vtek_log_error("vtek::descriptor_set_bind_uniform_buffer_dynamic(): {}",
	               "Not implemented!");
	return false;
}

bool vtek::descriptor_set_bind_storage_buffer_dynamic()
{
	vtek_log_error("vtek::descriptor_set_bind_storage_buffer_dynamic(): {}",
	               "Not implemented!");
	return false;
}

bool vtek::descriptor_set_bind_input_attachment()
{
	vtek_log_error("vtek::descriptor_set_bind_input_attachment(): {}",
	               "Not implemented!");
	return false;
}

bool vtek::descriptor_set_bind_inline_uniform_block()
{
	vtek_log_error("vtek::descriptor_set_bind_inline_uniform_block(): {}",
	               "Not implemented!");
	return false;
}
