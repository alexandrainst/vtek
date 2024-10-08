#pragma once

#include "vtek_object_handles.hpp"

#include "vtek_image.hpp"
#include "vtek_uniform_data.hpp"


namespace vtek
{
	VkDescriptorSet descriptor_set_get_handle(const DescriptorSet* set);

	// Apply the descriptors added/modified with the functions declared below.
	// This function will finalize these changes into the descriptor set,
	// and must be called before command buffer recording.
	void descriptor_set_update(DescriptorSet* set, Device* device);


	// ========================== //
	// === Update descriptors === //
	// ========================== //
	bool descriptor_set_bind_sampler();

	bool descriptor_set_bind_combined_image2d_sampler(
		DescriptorSet* set, uint32_t binding,
		Sampler* sampler, Image2D* image, ImageLayout imageLayout);

	bool descriptor_set_bind_sampled_image();

	bool descriptor_set_bind_uniform_texel_buffer();

	bool descriptor_set_bind_storage_texel_buffer();

	bool descriptor_set_bind_uniform_buffer(
		DescriptorSet* set, uint32_t binding,
		Buffer* buffer, UniformBufferType type);

	bool descriptor_set_bind_storage_buffer();

	bool descriptor_set_bind_uniform_buffer_dynamic();

	bool descriptor_set_bind_storage_buffer_dynamic();

	bool descriptor_set_bind_input_attachment();

	bool descriptor_set_bind_inline_uniform_block();
}
