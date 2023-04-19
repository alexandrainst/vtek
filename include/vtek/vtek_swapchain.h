#pragma once

#include <cstdint>

#include "vtek_physical_device.h"
#include "vtek_device.h"


namespace vtek
{
	struct SwapchainCreateInfo
	{
		bool vsync {false};

		// Will force a swapchain internal queue policy which will always render the newest image
		// regardless of queued images. This will result in lower render latency but increase
		// electricity consumption.
		// NOTE: Highly discouraged for mobile devices!
		bool prioritizeLowLatency {false};

		int framebufferWidth {0};
		int framebufferHeight {0};

		// TODO: This could be a neat feature, IFF window handling is implemented inside vtek.
		// NOTE: Other things, such as render passes, pipelines, framebuffers, etc., should also be re-created!
		// REVIEW: Perhaps then not after all?!
		//bool recreateOnWindowResize {false};
	};


	struct Swapchain; // opaque handle

	Swapchain* swapchain_create(
		const SwapchainCreateInfo* info, VkSurfaceKHR surface,
		const PhysicalDevice* physicalDevice, Device* device);
	bool swapchain_recreate(Swapchain* swapchain);
	void swapchain_destroy(Swapchain* swapchain, const Device* device);

	uint32_t swapchain_get_length(Swapchain* swapchain);

	VkImage swapchain_get_image(Swapchain* swapchain, uint32_t index);
	VkImageView swapchain_get_image_view(Swapchain* swapchain, uint32_t index);
	VkImage swapchain_get_image_format(Swapchain* swapchain);

	// Optionally returns an index which refers to a `VkImage` in the list
	// of swapchain images array, which can be used to pick the right data
	// for the frame. This can include looking up into arrays of
	// command buffers, uniform buffers, etc.
	bool swapchain_acquire_next_image_index(Swapchain* swapchain, uint32_t* outImageIndex);

	bool swapchain_present_image(Swapchain* swapchain, uint32_t presentImageIndex);
}
