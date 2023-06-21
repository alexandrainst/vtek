#include "vtek_vulkan.pch"
#include "vtek_descriptor_set_layout.hpp"

#include "vtek_logging.hpp"


/* struct implementation */
struct vtek::DescriptorSetLayout
{

};



/* interface */
vtek::DescriptorSetLayout* vtek::descriptor_set_layout_create(
	const DescriptorSetLayoutInfo* info, vtek::Device* device)
{
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
			return nullptr;
		}

		VkDescriptorSetLayoutBinding binding{};
		binding.binding = bindRef.binding;
		binding.descriptorType = type.value();
		binding.descriptorCount = bindRef.count;
		binding.stageFlags

		bindings.emplace_back();
	}

}

void vtek::descriptor_set_layout_destroy()
{

}
