#include "vtek_frame_sync.hpp"

#include <vector>

/* struct implementation */
struct vtek::FrameSync
{
	uint32_t numFramesInFlight {0U};
	uint32_t currentFrameIndex {0U};

	// Semaphores that will be signaled once a swapchain image becomes ready.
	// Used for acquiring swapchain images.
	VkSemaphore imageAvailableSemaphores[vtek::kMaxFramesInFlight];

	// Semaphores that will be signaled once rendering of a frame has completed.
	// Used for releasing swapchain images to the presentation queue.
	VkSemaphore renderFinishedSemaphores[vtek::kMaxFramesInFlight];

	// Before we can start drawing a frame, the CPU needs to wait for
	// the swapchain image to become ready. This is accomplished by
	// having this list of frames which guard the current frame
	// Before we can draw, we need to wait for fences guarding the current
	// swap chain image.
	VkFence inFlightFences[vtek::kMaxFramesInFlight];

	// This vector will be created to the length of the swapchain,
	// and for each swapchain image hold a pointer to a fence in
	// `inFlightFences`. Used for signalling when it's possible to
	// render to a swapchain image.
	std::vector<VkFence> imagesInFlight;
};


/* interface */
vtek::FrameSync* vtek::frame_sync_create(vtek::Device* device, vtek::Swapchain* swapchain)
{
	vtek_log_error("vtek::frame_sync_create -> Not implemented!");
	return nullptr;
}

void vtek::frame_sync_destroy(vtek::FrameSync* frameSync)
{
	vtek_log_error("vtek::frame_sync_destroy -> Not implemented!");
}

VkSemaphore vtek::frame_sync_get_current_signal_semaphore(vtek::FrameSync* frameSync)
{
	return frameSync->renderFinishedSemaphores[];
}

VkSemaphore vtek::frame_sync_get_current_wait_semaphore(vtek::FrameSync* frameSync)
{

}

bool vtek::frame_sync_wait_begin_frame(vtek::FrameSync* frameSync)
{
	vtek_log_error("vtek::frame_sync_wait_begin_frame -> Not implemented!");
	return false;
}

bool vtek::frame_sync_wait_image_ready(vtek::FrameSync* frameSync, uint32_t imageIndex)
{
	vtek_log_error("vtek::frame_sync_wait_image_ready -> Not implemented!");
	return false;

	// TODO: Also set image in use! (yggdrasil::graphics::vulkan_frame_sync_set_image_in_use)
}


// ============================ //
// === Simplified interface === //
// ============================ //
void vtek::frame_sync_reset(vtek::FrameSync* frameSync)
{

}

vtek::BeginFrameStatus vtek::frame_sync_begin_frame(vtek::FrameSync* frameSync)
{

}

void vtek::frame_sync_end_frame(vtek::FrameSync* frameSync)
{
	uint32_t& curFrame = frameSync->currentFrameIndex;
	curFrame = (curFrame + 1) % frameSync->numFramesInFlight;
}
