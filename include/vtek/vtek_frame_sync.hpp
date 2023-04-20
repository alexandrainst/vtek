#pragma once

#include <cstdint>
#include <vulkan/vulkan.h>


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


	struct FrameSync; // opaque handle

	FrameSync* frame_sync_create(Swapchain* swapchain);
	void frame_sync_destroy(FrameSync* frameSync);

	// Call this function before starting a new rendering frame. This
	// handles internal synchronization to correctly limit the amount
	// of frames that can be handled at the GPU at any time. Failure
	// to call this function at the beginning of each frame may result
	// in indefinite stalls or GPU memory corruption.
	//
	// Returns false if a new frame could not be started.
	bool frame_sync_wait_begin_frame(FrameSync* frameSync);

	// Before using the image as framebuffer attachment, we need to make
	// sure that no previous frame is still using this image.
	bool frame_sync_wait_image_ready(FrameSync* frameSync, uint32_t imageIndex);
}
