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

	enum class FormatDepthStencilTest
	{
		none, depth, stencil, depth_and_stencil
	};

	FormatDepthStencilTest get_format_depth_stencil_test(VkFormat format);
}
