#pragma once

#include <vulkan/vulkan.h>
#include <vector>

#include "vtek_descriptor_pool.hpp"
#include "vtek_shaders.hpp"
#include "vtek_types.hpp"
#include "vtek_vulkan_handles.hpp"


namespace vtek
{
	struct DescriptorLayoutBinding
	{
		// Specifies how the descriptor is used, e.g. as a uniform buffer
		// or a sampled image.
		DescriptorType type;

		// Binding index referenced in the shader.
		uint32_t binding {0U};

		// In which shader stages should the descriptor be visible.
		vtek::EnumBitmask<ShaderStage> shaderStages{};

		// If the shader variable represents an array of uniform buffer
		// objects, then this value may be greater than 1.
		// Could be used for small-scale instancing or bone transforms
		// for skeletal animation.
		uint32_t count {1U};
	};

	struct DescriptorSetLayoutInfo
	{
		std::vector<DescriptorLayoutBinding> bindings;
	};


	DescriptorSetLayout* descriptor_set_layout_create(
		const DescriptorSetLayoutInfo* info, Device* device);

	void descriptor_set_layout_destroy(
		DescriptorSetLayout* layout, Device* device);

	VkDescriptorSetLayout descriptor_set_layout_get_handle(
		DescriptorSetLayout* layout);
}
