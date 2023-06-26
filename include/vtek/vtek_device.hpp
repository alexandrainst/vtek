#pragma once

#include <vulkan/vulkan.h>

#include "vtek_vulkan_handles.hpp"
#include "vtek_vulkan_version.hpp"


namespace vtek
{
	struct DeviceCreateInfo
	{
		// TODO: Since these were specified during physical device pick, they should probably be removed from here!
		// === Extensions ===
		// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VK_KHR_maintenance1.html
		// Promoted to Vulkan 1.1 Core
		bool enableMaintenance1Extension {false}; // TODO: Core since Vulkan 1.1, so probably not needed
		bool enableRaytracingExtension {false};

		// === Queues ===
		// We can prefer to have a dedicated queue for transfer operations, which is different from
		// the graphics queue. This will require extra synchronization between resources shared by
		// different queues, but it may still be a desired choice of optimization for certain
		// applications.
		// Some physical devices have a dedicated transfer queue family, which will be used
		// when available (for the price of more external synchronization).
		// TODO: We could move this to physical device pick instead?!
		bool preferSeparateTransferQueue {false};

		bool preferPresentInComputeQueue {false};

		// "If the compute operations feed graphics operations, it's generally best to stick them
		// in the same queue."
		// https://community.khronos.org/t/guidelines-for-selecting-queues-and-families/7222
		// TODO: We could move this to physical device pick instead?!
		bool preferSeparateComputeQueue {false};
		// As most people recommend, we only need 1 graphics queue (likely with present support)
		// when we are doing rendering. But we are free to use any number of transfer and
		// compute queues.
		uint32_t numTransferQueues {1U};
		// NOTE: This field is ignored unless `preferSeparateComputeQueue` is set.
		uint32_t numComputeQueues {0U};

		// Features
		bool enableBindlessTextureSupport {false};

		// If this is set to false, an allocator must be manually created
		// aftwerwards and passed on to functions that create buffers and images.
		// NOTE: It is recommended to just leave this as is for simplicity.
		bool createDefaultAllocator {true};

		// A device will always have a command scheduler for running background
		// commands, such as single-use transfer operations. We can choose whether
		// this command scheduler runs on a parallel thread or not.
		bool asyncCommandScheduler {true};
	};

	struct DeviceExtensions
	{
		bool dynamicRendering {false};
		bool swapchain {false};
		bool getMemoryRequirements2 {false};
		bool rayTracingNV {false}; // TODO: This is only NVidia specific!
		// NEXT: Better!
		// NOTE: NV extension is aliased to KHR, so better just use the KHR!
		bool raytracing {false};
	};


	Device* device_create(
		const DeviceCreateInfo* info, const Instance* instance,
		const PhysicalDevice* physicalDevice);

	void device_destroy(Device* device);

	VkDevice device_get_handle(const Device* device);
	VkPhysicalDevice device_get_physical_handle(const Device* device);
	const VulkanVersion* device_get_vulkan_version(const Device* device);
	const DeviceExtensions* device_get_enabled_extensions(const Device* device);
	const VkPhysicalDeviceFeatures* device_get_enabled_features(const Device* device);

	Allocator* device_get_allocator(const Device* device);
	CommandScheduler* device_get_command_scheduler(const Device* device);

	// If any of these functions return `nullptr`, then no corresponding queues
	// were created.
	Queue* device_get_graphics_queue(Device* device);
	Queue* device_get_present_queue(Device* device);
	// TODO: Return std::vector<>&&, or is that too advanced?
	std::vector<Queue*> device_get_transfer_queues(Device* device);
	std::vector<Queue*> device_get_compute_queues(Device* device);
	bool device_get_graphics_present_same_family(const Device* device);

	void device_wait_idle(Device* device);


	// Obtain the maximum number of samples this device supports for any rendering
	// purposes. This value may be different for depth/stencil buffers than for
	// color buffers, so the returned value will be the minimum for the
	// corresponding queried buffer types.
	struct SampleCountQuery
	{
		bool color {false};
		bool depth {false};
		bool stencil {false};
	};

	VkSampleCountFlagBits device_get_max_sample_count(
		Device* device, const SampleCountQuery* query);
}
