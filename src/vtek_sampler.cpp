#include "vtek_vulkan.pch"
#include "vtek_sampler.hpp"

#include "vtek_device.hpp"
#include "vtek_logging.hpp"

using SAMode = vtek::SamplerAddressMode;
using SBColor = vtek::SamplerBorderColor;
using SDCompOp = vtek::SamplerDepthCompareOp;


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

static VkBorderColor get_border_color(vtek::SamplerBorderColor color)
{
	switch (color)
	{
	case SBColor::transparent_black_float:
		return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
	case SBColor::transparent_black_int:
		return VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;
	case SBColor::opaque_black_float:
		return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
	case SBColor::opaque_black_int:
		return VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	case SBColor::opaque_white_float:
		return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	case SBColor::opaque_white_int:
		return VK_BORDER_COLOR_INT_OPAQUE_WHITE;

	default:
		vtek_log_error("vtek_sampler.cpp: get_border_color(): {}",
		               "Unrecognized SamplerBorderColor enum value!");
		vtek_log_warn("Will default to transparent border color.");
		return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
	}
}

static VkCompareOp get_depth_compare_op(vtek::SamplerDepthCompareOp op)
{
	switch (op)
	{
	case SDCompOp::never:            return VK_COMPARE_OP_NEVER;
	case SDCompOp::less:             return VK_COMPARE_OP_LESS;
	case SDCompOp::equal:            return VK_COMPARE_OP_EQUAL;
	case SDCompOp::less_or_equal:    return VK_COMPARE_OP_LESS_OR_EQUAL;
	case SDCompOp::greater:          return VK_COMPARE_OP_GREATER;
	case SDCompOp::not_equal:        return VK_COMPARE_OP_NOT_EQUAL;
	case SDCompOp::greater_or_equal: return VK_COMPARE_OP_GREATER_OR_EQUAL;
	case SDCompOp::always:           return VK_COMPARE_OP_ALWAYS;
	default:
		vtek_log_error("vtek_sampler.cpp: get_depth_compare_op(): {}",
		               "Unrecognized CompareOp enum value!");
		vtek_log_warn("Will default to COMPARE_OP_NEVER.");
		return VK_COMPARE_OP_NEVER;
	}
}



/* interface */
vtek::Sampler* vtek::sampler_create(
	const vtek::SamplerInfo* info, vtek::Device* device)
{
	VkDevice dev = vtek::device_get_handle(device);

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

	// Anisotropic filtering
	if (info->anisotropicFiltering)
	{
		createInfo.anisotropyEnable = VK_TRUE;
		// Calculate the maximally supported value on the physical device
		float givenMax = info->maxAnisotropy.get();
		auto props = vtek::device_get_physical_properties(device);
		float realMax = props->limits.maxSamplerAnisotropy;
		createInfo.maxAnisotropy = (givenMax > realMax) ? realMax : givenMax;
	}
	else
	{
		createInfo.anisotropyEnable = VK_FALSE;
		createInfo.maxAnisotropy = 0.0f;
	}

	// Depth comparison OP
	if (info->depthCompareOp == vtek::SamplerDepthCompareOp::never)
	{
		createInfo.compareEnable = VK_FALSE;
		createInfo.compareOp = VK_COMPARE_OP_NEVER;
	}
	else
	{
		createInfo.compareEnable = VK_TRUE;
		createInfo.compareOp = get_depth_compare_op(info->depthCompareOp);
	}

	// May be used to clamp the computed LOD value.
	// NOTE: Not implemented yet here.
	createInfo.minLod = 0.0f;
	createInfo.maxLod = VK_LOD_CLAMP_NONE; // Avoid setting a maximum
	createInfo.borderColor = get_border_color(info->borderColor);

	// NOTE: We could allow for unnormalized texture coordinates in shaders,
	// but that comes with several limitations and requirements, and is
	// probably not required here, hence omitted.
	createInfo.unnormalizedCoordinates = VK_FALSE;

	VkResult result = vkCreateSampler(
		dev, &createInfo, nullptr, &sampler->vulkanHandle);
	if (result != VK_SUCCESS)
	{
		vtek_log_error("Failed to create sampler!");
		delete sampler;
		return nullptr;
	}

	return sampler;
}

void vtek::sampler_destroy(vtek::Sampler* sampler, vtek::Device* device)
{
	if (sampler == nullptr) { return; }

	VkDevice dev = vtek::device_get_handle(device);
	vkDestroySampler(dev, sampler->vulkanHandle, nullptr);

	*sampler = {};
	delete sampler;
}

VkSampler vtek::sampler_get_handle(const vtek::Sampler* sampler)
{
	return sampler->vulkanHandle;
}
