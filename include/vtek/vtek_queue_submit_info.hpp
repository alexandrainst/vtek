#pragma once

#include <vulkan/vulkan.h>


namespace vtek
{
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
			if (numWait >= kMaxSemaphores) { return; }
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
}
