#include "vtek_vulkan.pch"
#include "vtek_queue.hpp"

#include "vtek_command_buffer.hpp"

/* struct implementation */
#include "impl/vtek_queue_struct.hpp"



VkQueue vtek::queue_get_handle(const vtek::Queue* queue)
{
	return queue->vulkanHandle;
}

VkSharingMode vtek::queue_get_sharing_mode(
	const vtek::Queue* queue1, const vtek::Queue* queue2)
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

bool vtek::queue_submit(
	vtek::Queue* queue, vtek::CommandBuffer* commandBuffer,
	const vtek::SubmitInfo* submitInfo)
{
	VkCommandBuffer buf = vtek::command_buffer_get_handle(commandBuffer);

	const VkSubmitInfo info = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.pNext = nullptr,
		.waitSemaphoreCount = submitInfo->NumWaitSemaphores(),
		.pWaitSemaphores = submitInfo->WaitSemaphores(),
		.pWaitDstStageMask = submitInfo->WaitPipelineStages(),
		.commandBufferCount = 1,
		.pCommandBuffers = &buf,
		.signalSemaphoreCount = submitInfo->NumSignalSemaphores(),
		.pSignalSemaphores = submitInfo->SignalSemaphores()
	};

	VkResult result = vkQueueSubmit(
		queue->vulkanHandle, 1, &info, submitInfo->PostSignalFence());
	return result == VK_SUCCESS;
}
