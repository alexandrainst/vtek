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

	// TODO: We don't want these two functions, but let them be for now. Replacement is found below!
	bool queue_submit(Queue* queue, const std::vector<VkSubmitInfo>& submitInfos);
	bool queue_submit(Queue* queue, const std::vector<VkSubmitInfo>& submitInfos, VkFence postSignalFence);

	// TODO: Requires that we can obtain the command pool handle from the command buffer interface!
	class SubmitInfo
	{
	public:
		static constexpr uint32_t kMaxSemaphores = 4;

		// SET
		inline void SetPostSignalFence(VkFence fence) { postSignalFence = fence; }
		inline void AddSignalSemaphore(VkSemaphore sem) {
			if (numSignal >= kMaxSemaphores) { return; }
			signalSemaphores[numSignal++] = sem;
		}
		inline void AddWaitSemaphore(VkSemaphore sem, VkPipelineStageFlags stage) {
			if (numWaitSemaphores >= kMaxSemaphores) { return; }
			waitSemaphores[numWait] = sem;
			waitPipelineStages[numWait] = stage;
			numWait++;
		}

		// GET
		VkFence PostSignalFence() { return postSignalFence; }
		uint32_t NumSignalSemaphores() { return static_cast<uint32_t>(numSignal); }
		const VkSemaphore* SignalSemaphores()
		{
			return (numSignal > 0) ? signalSemaphores : nullptr;
		}

		uint32_t NumWaitSemaphores() { return static_cast<uint32_t>(numWait); }
		const VkSemaphore* WaitSemaphores()
		{
			return (numWait > 0) ? waitSemaphores : nullptr;
		}
		const VkPipelineStageFlags* WaitPipelineStages()
		{
			return (numWait > 0) ? waitPipelineStages : nullptr;
		}

	private:
		uint8_t numSignal {0U};
		VkSemaphore signalSemaphores[kMaxSemaphores];
		uint8_t numWait {0U};
		VkSemaphore waitSemaphores[kMaxSemaphores];
		VkPipelineStageFlags waitPipelineStages[kMaxSemaphores] {0};
		VkFence postSignalFence {VK_NULL_HANDLE};
	};

	bool queue_submit(Queue* queue, CommandBuffer* commandBuffer, const SubmitInfo* submitInfo);

	// TODO: Is this desirable?
	bool queue_supports_graphics(const Queue* queue);
	bool queue_supports_present(const Queue* queue);
	bool queue_supports_compute(const Queue* queue);
	bool queue_supports_sparse_binding(const Queue* queue);
}
