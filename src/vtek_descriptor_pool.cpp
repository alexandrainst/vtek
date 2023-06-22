#include "vtek_vulkan.pch"
#include "vtek_descriptor_pool.hpp"

#include "vtek_descriptor_type.hpp"
#include "vtek_device.hpp"
#include "vtek_logging.hpp"
#include "vtek_vulkan_version.hpp"

#include <optional>


/* struct implementation */
struct vtek::DescriptorPool
{
	VkDescriptorPool vulkanHandle {VK_NULL_HANDLE};
};



/* interface */
vtek::DescriptorPool* vtek::descriptor_pool_create(
	vtek::DescriptorPoolInfo* info, vtek::Device* device)
{
	// TODO: Placed here at the start to enable a custom allocator.
	vtek::DescriptorPool* pool = new vtek::DescriptorPool{};

	VkDescriptorPoolCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;

	if (info->allowIndividualFree)
	{
		// This flag specifies if individual descriptor sets can be freed.
		// Similar to command buffer pools. If this bit is set, then the
		// function `vkFreeDescriptorSets` may be called.
		createInfo.flags |= VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	}
	if (info->allowUpdateAfterBind)
	{
		// This flag specifies that descriptor sets allocated from this pool
		// can include bindings with this bit:
		// `VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT`.
		createInfo.flags |= VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
	}

	// NOTE: Specify descriptor pool sizes (??)
	std::vector<VkDescriptorPoolSize> poolSizes;
	const uint32_t numTypes = info->descriptorTypes.size();
	uint32_t maxSets = 0U;

	for (uint32_t i = 0; i < numTypes; i++)
	{
		vtek::DescriptorPoolType poolType = info->descriptorTypes[i];
		std::optional<VkDescriptorType> typeOpt =
			vtek::get_descriptor_type(poolType.type, device);
		if (!typeOpt.has_value())
		{
			vtek_log_error("Failed to get descriptor type -- {}",
			               "cannot create descriptor pool!");
			delete pool;
			return nullptr;
		}

		poolSizes.push_back({ typeOpt.value(), poolType.count });
		maxSets += poolType.count;
	}

	// Maximum number of descriptor sets that CAN be allocated from the pool.
	// TODO: Should we add margin for safety?
	createInfo.maxSets = maxSets;

	// An array of `VkDescriptorPoolSize` structures, each containing a
	// descriptor type and number of descriptors of that type to be allocated
	// in the pool.
	createInfo.poolSizeCount = poolSizes.size();
	createInfo.pPoolSizes = poolSizes.data();

	VkDevice dev = vtek::device_get_handle(device);
	VkResult result = vkCreateDescriptorPool(
		dev, &createInfo, nullptr, &pool->vulkanHandle);
	if (result != VK_SUCCESS)
	{
		vtek_log_error("Failed to create descriptor pool!");
		delete pool;
		return nullptr;
	}

	return pool;
}

void vtek::descriptor_pool_destroy(
	vtek::DescriptorPool* pool, vtek::Device* device)
{
	if (pool == nullptr) return;
	VkDevice dev = vtek::device_get_handle(device);

	vkDestroyDescriptorPool(dev, pool->vulkanHandle, nullptr);
	pool->vulkanHandle = VK_NULL_HANDLE;

	delete pool;
}

VkDescriptorPool vtek::descriptor_pool_get_handle(vtek::DescriptorPool* pool)
{
	return pool->vulkanHandle;
}
