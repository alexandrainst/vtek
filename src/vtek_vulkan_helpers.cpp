#include "vtek_vulkan_helpers.h"


bool vtek::findSupportedImageFormat(
	VkPhysicalDevice physicalDevice,
	std::vector<VkFormat> prioritizedCandidates,
	VkImageTiling tiling,
	VkFormatFeatureFlags features,
	VkFormat* outFormat)
{
	for (const VkFormat& format : prioritizedCandidates)
	{
		VkFormatProperties properties;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &properties);

		if (tiling == VK_IMAGE_TILING_LINEAR &&
		    (properties.linearTilingFeatures & features) == features)
		{
			*outFormat = format;
			return true;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
		         (properties.optimalTilingFeatures & features) == features)
		{
			*outFormat = format;
			return true;
		}
	}

	return false;
}
