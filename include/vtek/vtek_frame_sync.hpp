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
	static constexpr uint32_t kMaxFramesInFlight = 2;

	struct FrameSync; // opaque handle


}
