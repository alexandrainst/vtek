#pragma once

#include <cstdint>
#include <vulkan/vulkan.h>

#include "vtek_device.hpp"
#include "vtek_physical_device.hpp"
#include "vtek_queue.hpp"


namespace vtek
{
	// No more frames than this numbers shall be rendered on the GPU
	// at any given time. The actual number of frames in flight depends
	// on whether or not the swapchain was created for triple buffering.
	// If triple buffering then swapchain length is 3, and else 2.
	//
	// The number of _actual_ frames in flight is then obtained by
	// subtracting 1 from the swapchain length.
	//
	// NOTE: There is no performance gain in making a swapchain longer,
	// since we allow for max 2 simultaneous frames in flight. And even
	// if higher number of frames in flight were allowed, it would only
	// reduce rendering latency and increase memory usage.
	static constexpr uint32_t kMaxFramesInFlight = 2;

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
	VkFormat swapchain_get_image_format(Swapchain* swapchain);

	// Optionally returns an index which refers to a `VkImage` in the list
	// of swapchain images array, which can be used to pick the right data
	// for the frame. This can include looking up into arrays of
	// command buffers, uniform buffers, etc.
	bool swapchain_acquire_next_image_index(Swapchain* swapchain, uint32_t* outImageIndex);

	bool swapchain_present_image(Swapchain* swapchain, uint32_t presentImageIndex);


	// ======================== //
	// === Better interface === //
	// ======================== //

	// TODO: Simplified interface? :
	enum class BeginFrameStatus
	{
		ok,
		fence_timeout,
		swapchain_outofdate
	};

	// Call this function before starting a new rendering frame. This
	// handles internal synchronization to correctly limit the amount
	// of frames that can be handled at the GPU at any time. Failure
	// to call this function at the beginning of each frame may result
	// in indefinite stalls or GPU memory corruption.
	//
	// Returns false if a new frame could not be started.
	bool swapchain_wait_begin_frame(Swapchain* swapchain);

	bool swapchain_acquire_next_image(Swapchain* swapchain, uint32_t* imageIndex);

	// TODO: Special enum return status?
	// Before using the image as framebuffer attachment, we need to make
	// sure that no previous frame is still using this image.
	bool swapchain_wait_image_ready(Swapchain* swapchain, uint32_t imageIndex);

	void swapchain_fill_queue_submit_info(Swapchain* swapchain, SubmitInfo* submitInfo);

	// TODO: Special enum return status?
	bool swapchain_wait_end_frame(Swapchain* swapchain, uint32_t frameIndex);
}
