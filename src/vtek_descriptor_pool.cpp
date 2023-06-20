#include "vtek_vulkan.pch"
#include "vtek_descriptor_pool.hpp"

#include "vtek_device.hpp"
#include "vtek_logging.hpp"
#include "vtek_vulkan_version.hpp"

#include <optional>


using DType = vtek::DescriptorType;


/* struct implementation */
struct vtek::DescriptorPool
{
	VkDescriptorPool vulkanHandle {VK_NULL_HANDLE};
};


/* helper functions */
static std::optional<VkDescriptorType> get_descriptor_type(
	vtek::DescriptorType type, vtek::Device* device)
{
	switch (type)
	{
	case DType::sampler:
		return VK_DESCRIPTOR_TYPE_SAMPLER;
	case DType::combined_image_sampler:
		return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	case DType::sampled_image:
		return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	case DType::storage_image:
		return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	case DType::uniform_texel_buffer:
		return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
	case DType::storage_texel_buffer:
		return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
	case DType::uniform_buffer:
		return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	case DType::storage_buffer:
		return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	case DType::uniform_buffer_dynamic:
		return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	case DType::storage_buffer_dynamic:
		return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
	case DType::input_attachment:
		return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;

	case DType::inline_uniform_block:
		// Provided by VK_VERSION_1_3
		{
#if defined(VK_VERSION_1_3)
			vtek::VulkanVersion vv = *vtek::device_get_vulkan_version(device);
			if (vv.major() > 1 || vv.minor() >= 3)
			{
				return VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK;
			}
			else
			{
				vtek_log_error(
					"The device must be created with at least Vulkan 1.3 {}",
					"to support descriptors of type INLINE_UNIFORM_BLOCK!");
				return std::nullopt;
			}
#else
			vtek_log_error(
				"At least Vulkan 1.3 must be installed on the system {}",
				"to support descriptors of type INLINE_UNIFORM_BLOCK!");
			return std::nullopt;
#endif
		}
		break; // control should not reach here!

	case DType::acceleration_structure:
		// Provided by VK_KHR_acceleration_structure
		// TODO: Check that this extension is available AND enabled!
		return VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;

		/*
		// Provided by VK_NV_ray_tracing
		VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV = 1000165000,
		// Provided by VK_QCOM_image_processing
		VK_DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM = 1000440000,
		// Provided by VK_QCOM_image_processing
		VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM = 1000440001,
		// Provided by VK_EXT_mutable_descriptor_type
		VK_DESCRIPTOR_TYPE_MUTABLE_EXT = 1000351000,
		// Provided by VK_EXT_inline_uniform_block
		VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT = VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK,
		// Provided by VK_VALVE_mutable_descriptor_type
		VK_DESCRIPTOR_TYPE_MUTABLE_VALVE = VK_DESCRIPTOR_TYPE_MUTABLE_EXT,
		*/

	default:
		vtek_log_error("vtek:descriptor_pool.cpp: Invalid descriptor type!");
		return std::nullopt;
	}
}



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
			get_descriptor_type(poolType.type, device);
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
