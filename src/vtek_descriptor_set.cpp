#include "vtek_vulkan.pch"
#include "vtek_descriptor_set.hpp"

#include "impl/vtek_descriptor_set_struct.hpp"
#include "vtek_buffer.hpp"
#include "vtek_descriptor_pool.hpp"
#include "vtek_descriptor_set_layout.hpp"
#include "vtek_device.hpp"
#include "vtek_image.hpp"
#include "vtek_logging.hpp"
#include "vtek_sampler.hpp"

#include <deque>
#include <vector>


/* interface */
VkDescriptorSet vtek::descriptor_set_get_handle(const vtek::DescriptorSet* set)
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

bool vtek::descriptor_set_bind_combined_image2d_sampler(
	vtek::DescriptorSet* set, uint32_t binding,
	vtek::Sampler* sampler, vtek::Image2D* image, vtek::ImageLayout imageLayout)
{
	VkDescriptorImageInfo imageInfo{};
	imageInfo.imageLayout = vtek::get_image_layout(imageLayout);
	imageInfo.imageView = vtek::image2d_get_view_handle(image);
	imageInfo.sampler = vtek::sampler_get_handle(sampler);
	set->imageInfos.emplace_back(imageInfo);

	VkWriteDescriptorSet writeSet{};
	writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeSet.pNext = nullptr;
	writeSet.dstSet = set->vulkanHandle;
	writeSet.dstBinding = binding; // TODO: Is this binding valid?

	writeSet.dstArrayElement = 0;
	writeSet.descriptorCount = 1; // Number of elements in `pImageInfo`.
	writeSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	writeSet.pImageInfo = &(set->imageInfos.back());
	writeSet.pBufferInfo = nullptr;
	writeSet.pTexelBufferView = nullptr;

	set->writeDescriptors.emplace_back(writeSet);

	return true;
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
