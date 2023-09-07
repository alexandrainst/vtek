#pragma once

#include <cstdint>
#include <vulkan/vulkan.h>


namespace vtek
{
	// Creates a human-readable interface for the packed `apiVersion` type
	// used many places through the Vulkan Api.
	class VulkanVersion
	{
	public:
		constexpr VulkanVersion(uint32_t vulkanApiVersion)
			: mVulkanApi(vulkanApiVersion),
			  mMajor(VK_API_VERSION_MAJOR(vulkanApiVersion)),
			  mMinor(VK_API_VERSION_MINOR(vulkanApiVersion)),
			  mPatch(VK_API_VERSION_PATCH(vulkanApiVersion))
		{}
		constexpr VulkanVersion(uint32_t major, uint32_t minor, uint32_t patch)
			: mVulkanApi(VK_MAKE_API_VERSION(0, major, minor, patch)),
			  mMajor(major), mMinor(minor), mPatch(patch)
		{}

		inline uint32_t apiVersion() const { return mVulkanApi; }
		inline uint32_t major() const { return mMajor; }
		inline uint32_t minor() const { return mMinor; }
		inline uint32_t patch() const { return mPatch; }

		constexpr bool operator>= (const VulkanVersion vv) const
		{
			return mVulkanApi >= vv.mVulkanApi;
		}

	private:
		uint32_t mVulkanApi;
		uint32_t mMajor;
		uint32_t mMinor;
		uint32_t mPatch;
	};

	// NOTE: Unit-testing of comparison operators. Un-comment to verify!
	// NOTE: If any of these tests fails to compile, vtek will likely break!
	// TODO: Create unit-test project and place there instead!
	// static_assert(VulkanVersion(1,0,0) >= VulkanVersion(1,0,0));

	// static_assert(VulkanVersion(1,1,0) >= VulkanVersion(1,0,0));
	// static_assert(VulkanVersion(1,1,0) >= VulkanVersion(1,1,0));

	// static_assert(VulkanVersion(1,2,0) >= VulkanVersion(1,0,0));
	// static_assert(VulkanVersion(1,2,0) >= VulkanVersion(1,1,0));
	// static_assert(VulkanVersion(1,2,0) >= VulkanVersion(1,2,0));

	// static_assert(VulkanVersion(1,3,0) >= VulkanVersion(1,0,0));
	// static_assert(VulkanVersion(1,3,0) >= VulkanVersion(1,1,0));
	// static_assert(VulkanVersion(1,3,0) >= VulkanVersion(1,2,0));
	// static_assert(VulkanVersion(1,3,0) >= VulkanVersion(1,3,0));
}
