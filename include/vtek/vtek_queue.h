#pragma once

#include <vulkan/vulkan.h>

#include <cstdint>
#include <vector>

namespace vtek
{
	struct Queue; // opaque handle


	VkQueue queue_get_handle(const Queue* queue);

	// REVIEW: It could make sense to create an enum wrapper around the return value
	//         since `VkSharingMode` is an untyped enum and thus not type safe.
	VkSharingMode queue_get_sharing_mode(const Queue* queue1, const Queue* queue2);

	bool queue_is_same_family(const Queue* queue1, const Queue* queue2);

	uint32_t queue_get_family_index(const Queue* queue);

	void queue_wait_idle(const Queue* queue);

	bool queue_submit(Queue* queue, const std::vector<VkSubmitInfo>& submitInfos);

	bool queue_submit(Queue* queue, const std::vector<VkSubmitInfo>& submitInfos, VkFence postSignalFence);

	// TODO: Is this desirable?
	bool queue_supports_graphics(const Queue* queue);
	bool queue_supports_present(const Queue* queue);
	bool queue_supports_compute(const Queue* queue);
	bool queue_supports_sparse_binding(const Queue* queue);
}