#pragma once

#include <vulkan/vulkan.h>
#include <cstdint>


namespace vtek
{
	class VulkanVersion
	{
	public:
		VulkanVersion(uint32_t vulkanApiVersion)
			: mVulkanApi(vulkanApiVersion),
			  mMajor(VK_API_VERSION_MAJOR(vulkanApiVersion)),
			  mMinor(VK_API_VERSION_MINOR(vulkanApiVersion)),
			  mPatch(VK_API_VERSION_PATCH(vulkanApiVersion))
		{}
		VulkanVersion(uint32_t major, uint32_t minor, uint32_t patch)
			: mVulkanApi(VK_MAKE_API_VERSION(0, major, minor, patch)),
			  mMajor(major), mMinor(minor), mPatch(patch)
		{}

		inline uint32_t apiVersion() { return mVulkanApi; }
		inline uint32_t major() { return mMajor; }
		inline uint32_t minor() { return mMinor; }
		inline uint32_t patch() { return mPatch; }

	private:
		uint32_t mVulkanApi;
		uint32_t mMajor;
		uint32_t mMinor;
		uint32_t mPatch;
	};
}
