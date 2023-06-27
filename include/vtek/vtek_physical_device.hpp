#pragma once

#include <cstdint>
#include <vector>
#include <vulkan/vulkan.h>

#include "vtek_types.hpp"
#include "vtek_vulkan_handles.hpp"


namespace vtek
{
	// Specifies if a descriptor may be updated while being bound to
	// command buffer without invalidating that command buffer.
	// The command buffer must not be pending execution.
	// NOTE: >= Vulkan 1.2 is required for enabling these features.
	enum class UpdateAfterBindFeature : uint32_t
	{
		uniform_buffer       = 0x0001U,
		sampled_image        = 0x0002U,
		storage_image        = 0x0004U,
		storage_buffer       = 0x0008U,
		uniform_texel_buffer = 0x0010U,
		storage_texel_buffer = 0x0020U
	};

	struct PhysicalDeviceInfo
	{
		// required properties
		uint64_t requiredMinMemoryBytes {0UL}; // TODO: This field is not implemented.

		// required queue family properties
		bool requireGraphicsQueue {false};
		bool requirePresentQueue {false};
		bool requireComputeQueue {false};
		bool requireSparseBindingQueue {false}; // TODO: This probably needs to be more in-detail

		// required features, e.g. geometry shader or uint16 shader types,
		// verified here and enabled later during device creation.
		VkPhysicalDeviceFeatures requiredFeatures {};

		// Various types deduced from VkPhysicalDeviceDescriptorIndexingFeatures:
		EnumBitmask<UpdateAfterBindFeature> updateAfterBindFeatures {};

		// required extensions
		bool requireRaytracingSupport {false};
		bool requireSwapchainSupport {false};
		bool requireDynamicRendering {false};
	};

	// TODO: Since this is only needed by device creation, maybe place it somewhere else?
	struct PhysicalDeviceQueueSupport
	{
		// queue flag support (transfer is always supported)
		bool graphics {false};
		bool present {false};
		bool compute {false};

		// queue support to be enabled, ie. which queues will be created
		bool graphicsRequired {false};
		bool presentRequired {false};
		bool computeRequired {false};

		uint32_t graphicsIndex {0};
		uint32_t graphicsMaxCount {0};
		bool graphicsHasPresent {false};
		bool graphicsHasCompute {false};
		bool graphicsHasSparseBinding {false};
		bool graphicsTimestampQuery {false};

		bool hasSeparateTransferFamily {false};
		uint32_t separateTransferIndex {0};
		uint32_t separateTransferMaxCount {0};
		bool separateTransferTimestampQuery {false};
		bool separateTransferHasSparseBinding {false};

		bool hasSeparateComputeFamily {false};
		uint32_t separateComputeIndex {0};
		uint32_t separateComputeMaxCount {0};
		bool separateComputeTimestampQuery {false};
		bool separateComputeHasSparseBinding {false};
		bool separateComputeHasPresent {false};
	};

	struct PhysicalDeviceExtensionSupport
	{
		// extension support
		bool dynamicRendering {false};
		bool raytracing {false};
		bool swapchain {false};
	};


	// For non-graphical Vulkan applications we can omit providing a surface handle,
	// and skip the steps checking for presentation queue and swapchain support.
	// NOTE: Even with "compute-only" apps it might still be desirable to run graphics commands.
	// NOTE: The choice of `info->requireGraphicsSupport` will determine whether any graphics
	//       queues are created later during device creation, so this choice will have no
	//       performance indications regardless of hardware.
	PhysicalDevice* physical_device_pick(
		const PhysicalDeviceInfo* info, const Instance* instance);

	PhysicalDevice* physical_device_pick(
		const PhysicalDeviceInfo* info, const Instance* instance, VkSurfaceKHR surface);

	void physical_device_release(PhysicalDevice* physicalDevice);

	VkPhysicalDevice physical_device_get_handle(const PhysicalDevice* physicalDevice);

	const VkPhysicalDeviceProperties* physical_device_get_properties(
		const PhysicalDevice* physicalDevice);

	const VkPhysicalDeviceMemoryProperties* physical_device_get_memory_properties(
		const PhysicalDevice* physicalDevice);

	const VkPhysicalDeviceFeatures* physical_device_get_required_features(
		const PhysicalDevice* physicalDevice);

	const std::vector<const char*>& physical_device_get_required_extensions(
		const PhysicalDevice* physicalDevice);

	const PhysicalDeviceQueueSupport* physical_device_get_queue_support(
		const PhysicalDevice* physicalDevice);

	const PhysicalDeviceExtensionSupport* physical_device_get_extension_support(
		const PhysicalDevice* physicalDevice);
}
