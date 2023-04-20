#include "vtek_frame_sync.hpp"

/* struct implementation */
struct vtek::FrameSync
{
	uint32_t numFramesInFlight {0U};

	// Semaphores that will be signaled once a swapchain image becomes ready.
	// Used for acquiring swapchain images.
	VkSemaphore imageAvailableSemaphores[vtek::kMaxFramesInFlight];

	// Semaphores that will be signaled once rendering of a frame has completed.
	// Used for releasing swapchain images to the presentation queue.
	VkSemaphore renderFinishedSemaphores[vtek::kMaxFramesInFlight];

	VkFence inFlightFences[vtek::kMaxFramesInFlight];
};


/* interface */
