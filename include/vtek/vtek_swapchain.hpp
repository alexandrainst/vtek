#pragma once

#include <cstdint>
#include <vulkan/vulkan.h>

#include "vtek_submit_info.hpp"
#include "vtek_vulkan_handles.hpp"


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

	struct SwapchainInfo
	{
		bool vsync {false};

		// Will force a swapchain internal queue policy which will always render the newest image
		// regardless of queued images. This will result in lower render latency but increase
		// electricity consumption.
		// NOTE: Highly discouraged for mobile devices!
		bool prioritizeLowLatency {false};

		uint32_t framebufferWidth {0};
		uint32_t framebufferHeight {0};

		// If no deferred technique is used, default depth buffers may be
		// created with and managed by the swapchain. This should be disabled
		// if render passes together with swapchain framebuffers are used, and
		// might be enabled if dynamic rendering is used.
		bool createDepthBuffers {false};
	};


	Swapchain* swapchain_create(
		const SwapchainInfo* info, VkSurfaceKHR surface,
		const PhysicalDevice* physicalDevice, Device* device);
	bool swapchain_recreate(
		Swapchain* swapchain, Device* device, VkSurfaceKHR surface,
		uint32_t framebufferWidth, uint32_t framebufferHeight);
	void swapchain_destroy(Swapchain* swapchain, const Device* device);

	uint32_t swapchain_get_length(Swapchain* swapchain);

	VkImage swapchain_get_image(Swapchain* swapchain, uint32_t index);
	VkImageView swapchain_get_image_view(Swapchain* swapchain, uint32_t index);
	VkFormat swapchain_get_image_format(Swapchain* swapchain);
	VkExtent2D swapchain_get_image_extent(const Swapchain* swapchain);


	// A status enum, returned by the frame functions below, to keep a client
	// application informed on when a swapchain should be re-created, or if
	// an error occured that requires process termination.
	enum class SwapchainStatus
	{
		ok, error, timeout, outofdate
	};

	// Call this function before starting a new rendering frame. This
	// handles internal synchronization to correctly limit the amount
	// of frames that can be handled at the GPU at any time. Failure
	// to call this function at the beginning of each frame may result
	// in indefinite stalls or GPU memory corruption.
	SwapchainStatus swapchain_wait_begin_frame(
		Swapchain* swapchain, Device* device, uint64_t timeout = UINT64_MAX);

	// Optionally returns an index which refers to a `VkImage` in the list
	// of swapchain images array, which can be used to pick the right data
	// for the frame. This can include looking up into arrays of
	// command buffers, uniform buffers, etc.
	SwapchainStatus swapchain_acquire_next_image(
		Swapchain* swapchain, Device* device,
		uint32_t* imageIndex, uint64_t timeout = UINT64_MAX);

	// Before using the image as framebuffer attachment, we need to make
	// sure that no previous frame is still using this image.
	SwapchainStatus swapchain_wait_image_ready(
		Swapchain* swapchain, Device* device,
		uint32_t imageIndex, uint64_t timeout = UINT64_MAX);

	// Before submitting work to a queue which renders into the swapchain image, the
	// queue needs to know which semaphore to wait on, and which semaphore to signal
	// after rendering is completed. This function will provide these semaphores and
	// return them inside the `submitInfo` argument, as well as a post-signal fence
	// to synchronize the CPU with the rendered frame.
	void swapchain_fill_queue_submit_info(Swapchain* swapchain, SubmitInfo* submitInfo);

	// This function should be called after a rendering workload has been submitted
	// to a queue which targets a swapchain image. It will instruct the presentation
	// queue, to which the swapchain internally stores a handle, to wait for the
	// rendering queue to finishe execution before presenting the frame.
	SwapchainStatus swapchain_present_frame(Swapchain* swapchain, uint32_t frameIndex);


	// Explicit image barriers for using the swapchain images in command buffers.
	// NOTE: Only use these functions wherever dynamic rendering is applied, and
	// use them as the first and last step, respectively, in the recording sequence.
	// NOTE: For rendering setups with render passes and swapchain framebuffers,
	// this step should be handled with subpass dependencies instead.
	void swapchain_dynamic_rendering_begin(
		Swapchain* swapchain, uint32_t imageIndex, CommandBuffer* commandBuffer,
		glm::vec3 clearColor);

	void swapchain_dynamic_rendering_end(
		Swapchain* swapchain, uint32_t imageIndex, CommandBuffer* commandBuffer);
}
