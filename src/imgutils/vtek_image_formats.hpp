#pragma once

#include <cstdint>
#include <vulkan/vulkan.h>

#include "vtek_format_support.hpp"
#include "vtek_image.hpp"
#include "vtek_types.hpp"


namespace vtek
{
	// ====================== //
	// === Format queries === //
	// ====================== //

	VkFormatFeatureFlags get_format_features(EnumBitmask<FormatFeature> features);

	// Find first image format in the list that is supported by the
	// physical device. Returns a boolean indicating success.
	bool find_supported_image_format(
		VkPhysicalDevice physicalDevice,
		std::vector<VkFormat> prioritizedCandidates,
		VkImageTiling tiling,
		EnumBitmask<FormatFeature> featureFlags,
		VkFormat* outFormat);

	// Find a suitable format matching provided parameters.
	// This function returns `VK_FORMAT_UNDEFINED` in case a match wasn't found.
	// TODO: Better return bool instead?
	VkFormat get_format_color(
		const FormatInfo* info, VkPhysicalDevice physDev,
		EnumBitmask<FormatFeature> featureFlags);

	enum class FormatDepthStencilTest
	{
		none, depth, stencil, depth_and_stencil
	};

	FormatDepthStencilTest get_format_depth_stencil_test(VkFormat format);
}
