#pragma once

#include "vtek_uniform_data.hpp"
#include "vtek_vulkan_handles.hpp"


namespace vtek
{
	DescriptorSet* descriptor_set_create(
		DescriptorPool* pool, DescriptorSetLayout* layout, Device* device);

	// TODO: Has pool been created with destroy individual flag?
	void descriptor_set_destroy(DescriptorSet* set);

	// Apply the descriptors added/modified with the functions declared below.
	// This function will finalize these changes into the descriptor set,
	// and must be called before command buffer recording.
	void descriptor_set_apply_updates(DescriptorSet* set, Device* device);


	// ========================== //
	// === Update descriptors === //
	// ========================== //

	bool descriptor_set_add_sampler();

	bool descriptor_set_add_combined_image_sampler();

	bool descriptor_set_add_sampled_image();

	bool descriptor_set_add_uniform_texel_buffer();

	bool descriptor_set_add_storage_texel_buffer();

	bool descriptor_set_add_uniform_buffer(
		DescriptorSet* set, uint32_t binding,
		Buffer* buffer, UniformBufferType type);

	bool descriptor_set_add_storage_buffer();

	bool descriptor_set_add_uniform_buffer_dynamic();

	bool descriptor_set_add_storage_buffer_dynamic();

	bool descriptor_set_add_input_attachment();

	bool descriptor_set_add_inline_uniform_block();
}
