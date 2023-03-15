#include <vulkan/vulkan.h>
#include <vector>

namespace vtek
{
	// Find first image format in the list that is supported by the
	// physical device. Returns a boolean indicating success.
	bool findSupportedImageFormat(
		VkPhysicalDevice physicalDevice,
		std::vector<VkFormat> prioritizedCandidates,
		VkImageTiling tiling,
		VkFormatFeatureFlags features,
		VkFormat* outFormat);
}
