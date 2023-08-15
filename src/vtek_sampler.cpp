#include "vtek_vulkan.pch"
#include "vtek_sampler.hpp"

#include "vtek_device.hpp"
#include "vtek_logging.hpp"

using SAMode = vtek::SamplerAddressMode;


/* struct implementation */
struct vtek::Sampler
{
	VkSampler vulkanHandle {VK_NULL_HANDLE};
};



/* helper functions */
static VkSamplerAddressMode get_address_mode(
	vtek::SamplerAddressMode mode, vtek::Device* device)
{
	switch (mode)
	{
	case SAMode::repeat:          return VK_SAMPLER_ADDRESS_MODE_REPEAT;
	case SAMode::mirrored_repeat: return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
	case SAMode::clamp_to_edge:   return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	case SAMode::clamp_to_border: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	case SAMode::mirror_clamp_to_edge:
		// Dual checks: Supported instance version, and actual created version.
#if defined(VK_API_VERSION_1_2)
		if (vtek::device_get_vulkan_version(device) >= vtek::VulkanVersion(1, 2, 0))
		{
			return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
		}
		return VK_SAMPLER_ADDRESS_MODE_REPEAT;
#else
		vtek_log_error(
			"vtek_sampler.cpp: get_address_mode(): {}",
			"Vulkan >= 1.2 must be installed to use mirror_clamp_to_edge!");
		vtek_log_warn("Will fallback to SamplerAddressMode::repeat!");
		return VK_SAMPLER_ADDRESS_MODE_REPEAT;
#endif
	default:
		return VK_SAMPLER_ADDRESS_MODE_REPEAT;
	}
}



/* interface */
vtek::Sampler* vtek::sampler_create(
	const vtek::SamplerInfo* info, vtek::Device* device)
{
	auto sampler = new vtek::Sampler;

	VkSamplerCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.magFilter =
		(info->magFilter == vtek::SamplerFilterMode::linear)
		? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
	createInfo.minFilter =
		(info->minFilter == vtek::SamplerFilterMode::linear)
		? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
	createInfo.mipmapMode =
		(info->mipmapFilter == vtek::SamplerFilterMode::linear)
		? VK_SAMPLER_MIPMAP_MODE_LINEAR : VK_SAMPLER_MIPMAP_MODE_NEAREST;

	VkSamplerAddressMode addrMode = get_address_mode(info->addressMode, device);
	createInfo.addressModeU = addrMode;
	createInfo.addressModeV = addrMode;
	createInfo.addressModeW = addrMode;

	// TODO: What is the difference between this and `baseMipLevel` in image views ?
	createInfo.mipLodBias = 0.0f;

	createInfo.anisotropyEnable = VulkanBool(info->anisotropicFiltering).get();
	createInfo.maxAnisotropy = info->maxAnisotropy.get();

	// TODO: Probably, `compareOp` is only used for depth/stencil images!
	// if (info->compareOp == )
	// {
	// 	createInfo.compareEnable = 
	// }
	// else
	// {
	// 	createInfo.compareEnable = 
	// }

	vtek_log_error("sampler_create(): Not implemented!");
	return nullptr;
}

void vtek::sampler_destroy(vtek::Sampler* sampler)
{
	//sampler->vulkanHandle;      
}

VkSampler vtek::sampler_get_handle(const vtek::Sampler* sampler)
{
	return sampler->vulkanHandle;
}
