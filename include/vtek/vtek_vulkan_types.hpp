#pragma once

#include <vulkan/vulkan.h>


namespace vtek
{
	// Multisampling is one way to perform anti-aliasing, by sampling multiple
	// fragments per-pixel, and then resolving those fragments. This smooths out
	// polygon edges. Multisampling is not recommended for deferred rendering.
	enum class MultisampleType
	{
		none, msaa_x2, msaa_x4, msaa_x8, msaa_x16, msaa_x32, msaa_x64
	};

	VkSampleCountFlagBits get_multisample_count(MultisampleType sample);

	// Cull mode determines if any face of rendered polygons should be culled
	// (ie. discarded) by pipeline rasterization. Back culling is recommended
	// to avoid overdraw.
	enum class CullMode
	{
		none, front, back, front_and_back
	};

	VkCullModeFlags get_cull_mode(CullMode mode);
}
