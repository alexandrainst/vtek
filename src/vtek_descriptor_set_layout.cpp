#include "vtek_vulkan.pch"
#include "vtek_descriptor_set_layout.hpp"

#include "vtek_device.hpp"
#include "vtek_logging.hpp"


/* struct implementation */
struct vtek::DescriptorSetLayout
{
	VkDescriptorSetLayout vulkanHandle {VK_NULL_HANDLE};
};



/* interface */
vtek::DescriptorSetLayout* vtek::descriptor_set_layout_create(
	const DescriptorSetLayoutInfo* info, vtek::Device* device)
{
	VkDevice dev = vtek::device_get_handle(device);

	// Place at beginning to allow for custom allocator
	auto layout = new vtek::DescriptorSetLayout;

	std::vector<VkDescriptorSetLayoutBinding> bindings;

	for (uint32_t i = 0; i < info->bindings.size(); i++)
	{
		const vtek::DescriptorLayoutBinding& bindRef = info->bindings[i];

		std::optional<VkDescriptorType> type =
			vtek::get_descriptor_type(bindRef.type, device);
		if (!type.has_value())
		{
			vtek_log_error("Invalid descriptor type -- {}",
			               "cannot create descriptor set layout!");
			delete layout;
			return nullptr;
		}

		VkDescriptorSetLayoutBinding binding{};
		binding.binding = bindRef.binding;
		binding.descriptorType = type.value();
		binding.descriptorCount = bindRef.count;
		binding.stageFlags = vtek::get_shader_stage_flags(bindRef.shaderStages);
		// NOTE: Immutable samplers are, per choice, not supported.
		// This may change.
		binding.pImmutableSamplers = nullptr;

		bindings.emplace_back(binding);
	}

	VkDescriptorSetLayoutCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0U; // TODO: ?
	createInfo.bindingCount = bindings.size();
	createInfo.pBindings = bindings.data();

	VkResult result = vkCreateDescriptorSetLayout(
		dev, &createInfo, nullptr, &layout->vulkanHandle);
	if (result != VK_SUCCESS)
	{
		vtek_log_error("Failed to create descriptor set layout!");
		delete layout;
		return nullptr;
	}

	return layout;
}

void vtek::descriptor_set_layout_destroy(
	vtek::DescriptorSetLayout* layout, vtek::Device* device)
{
	if (layout == nullptr) return;

	VkDevice dev = vtek::device_get_handle(device);

	vkDestroyDescriptorSetLayout(dev, layout->vulkanHandle, nullptr);
	layout->vulkanHandle = VK_NULL_HANDLE;

	delete layout;
}

VkDescriptorSetLayout vtek::descriptor_set_layout_get_handle(
	vtek::DescriptorSetLayout* layout)
{
	return layout->vulkanHandle;
}
