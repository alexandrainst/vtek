#include "vtek_queue.hpp"


/* struct implementation */
#include "impl/vtek_queue_struct.hpp"



VkQueue vtek::queue_get_handle(const vtek::Queue* queue)
{
	return queue->vulkanHandle;
}

VkSharingMode vtek::queue_get_sharing_mode(const vtek::Queue* queue1, const vtek::Queue* queue2)
{
	return (queue1->familyIndex == queue2->familyIndex)
		? VK_SHARING_MODE_CONCURRENT
		: VK_SHARING_MODE_EXCLUSIVE;
}

bool vtek::queue_is_same_family(const vtek::Queue* queue1, const vtek::Queue* queue2)
{
	return queue1->familyIndex == queue2->familyIndex;
}

uint32_t vtek::queue_get_family_index(const vtek::Queue* queue)
{
	return queue->familyIndex;
}

void vtek::queue_wait_idle(const vtek::Queue* queue)
{
	vkQueueWaitIdle(queue->vulkanHandle);
}

bool vtek::queue_submit(vtek::Queue* queue, const std::vector<VkSubmitInfo>& submitInfos)
{
	VkResult result = vkQueueSubmit(queue->vulkanHandle, submitInfos.size(), submitInfos.data(), VK_NULL_HANDLE);
	return result == VK_SUCCESS;
}

bool vtek::queue_submit(vtek::Queue* queue, const std::vector<VkSubmitInfo>& submitInfos, VkFence postSignalFence)
{
	VkResult result = vkQueueSubmit(queue->vulkanHandle, submitInfos.size(), submitInfos.data(), postSignalFence);
	return result == VK_SUCCESS;
}

bool vtek::queue_submit(
	vtek::Queue* queue, vtek::CommandBuffer* commandBuffer, vtek::FrameSync* frameSync)
{
	VkSubmitInfo submitInfo = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.commandBufferCount = 1,
		.pCommandBuffers = commandBuffer->vulkanHandle,
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = vtek::frame_sync_get_current_signal_semaphore(frameSync), // TODO: Create this function!
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = vtek::frame_sync_get_current_wait_semaphore(frameSync),
		.pWaitDstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT // TODO: Function input?
	};
}

bool vtek::queue_supports_graphics(const vtek::Queue* queue)
{
	return queue->queueFlags & VK_QUEUE_GRAPHICS_BIT;
}

bool vtek::queue_supports_present(const vtek::Queue* queue)
{
	return queue->presentSupport;
}

bool vtek::queue_supports_compute(const vtek::Queue* queue)
{
	return queue->queueFlags & VK_QUEUE_COMPUTE_BIT;
}

bool vtek::queue_supports_sparse_binding(const vtek::Queue* queue)
{
	return queue->queueFlags & VK_QUEUE_SPARSE_BINDING_BIT;
}
