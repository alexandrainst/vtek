#pragma once

#include <vulkan/vulkan.h>
#include "vtek_instance.h"

#include <cstdint>
#include <vector>

namespace vtek
{
	struct PhysicalDeviceInfo
	{
		// required properties
		uint64_t requiredMinMemoryBytes {0UL}; // TODO: This field is not implemented.

		// required queue familily properties
		bool requireGraphicsQueue {false};
		bool requirePresentQueue {false};
		bool requireComputeQueue {false};
		bool requireSparseBindingQueue {false}; // TODO: This probably needs to be more in-detail

		// required features, e.g. geometry shader or uint16 shader types,
		// verified here and enabled later during device creation.
		VkPhysicalDeviceFeatures requiredFeatures {};

		// required extensions
		bool requireRaytracingSupport {false};
		bool requireSwapchainSupport {false};
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
		bool raytracing {false};
		bool swapchain {false};
	};


	struct PhysicalDevice; // opaque handle

	// For non-graphical Vulkan applications we can omit providing a surface handle,
	// and skip the steps checking for presentation queue and swapchain support.
	// NOTE: Even with "compute-only" apps it might still be desirable to run graphics commands.
	// NOTE: The choice of `info->requireGraphicsSupport` will determine whether any graphics
	//       queues are created later during device creation, so this choice will have no
	//       performance indications regardless of hardware.
	PhysicalDevice* physical_device_pick(const PhysicalDeviceInfo* info, const Instance* instance);

	PhysicalDevice* physical_device_pick(
		const PhysicalDeviceInfo* info, const Instance* instance, VkSurfaceKHR surface);

	void physical_device_release(PhysicalDevice* physicalDevice);

	VkPhysicalDevice physical_device_get_handle(const PhysicalDevice* physicalDevice);

	const VkPhysicalDeviceProperties* physical_device_get_properties(const PhysicalDevice* physicalDevice);

	const VkPhysicalDeviceMemoryProperties* physical_device_get_memory_properties(
		const PhysicalDevice* physicalDevice);

	const VkPhysicalDeviceFeatures* physical_device_get_required_features(const PhysicalDevice* physicalDevice);

	const std::vector<const char*>& physical_device_get_required_extensions(
		const PhysicalDevice* physicalDevice);

	const PhysicalDeviceQueueSupport* physical_device_get_queue_support(const PhysicalDevice* physicalDevice);

	const PhysicalDeviceExtensionSupport* physical_device_get_extension_support(
		const PhysicalDevice* physicalDevice);
}
