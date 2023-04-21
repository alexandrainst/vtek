#pragma once

#include <cstdint>
#include <vector>
#include <utility>
#include <vulkan/vulkan.h>

#include "vtek_command_buffer.hpp"


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

	// TODO: Is this desirable?
	bool queue_supports_graphics(const Queue* queue);
	bool queue_supports_present(const Queue* queue);
	bool queue_supports_compute(const Queue* queue);
	bool queue_supports_sparse_binding(const Queue* queue);


	// TODO: We don't want these two functions, but let them be for now. Replacement is found below!
	bool queue_submit(Queue* queue, const std::vector<VkSubmitInfo>& submitInfos);
	bool queue_submit(Queue* queue, const std::vector<VkSubmitInfo>& submitInfos, VkFence postSignalFence);

	bool queue_submit(Queue* queue, CommandBuffer* commandBuffer, const SubmitInfo* submitInfo);
}
