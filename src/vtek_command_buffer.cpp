#include "vtCommandBuffer.h"
#include "vtek_logging.h"

using CBState = vtek::CommandBufferStateType;


/* struct implementation */
struct vtek::CommandBuffer
{
	VkCommandBuffer vulkanHandle {VK_NULL_HANDLE};
	CBState state {CBState::invalid};

	// If command buffer was created from a pool that was created with the
	// VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT flag. Otherwise the
	// command buffer cannot be reset.
	bool supportsReset {false};

	bool isSecondary {false}; // TODO: What to use this for?
};

vtek::CommandBuffer* vtek::command_buffer_create()
{

}

void vtek::command_buffer_destroy(vtek::CommandBuffer* commandBuffer)
{

}

bool vtek::command_buffer_reset(vtek::CommandBuffer* commandBuffer)
{
	// TODO: How do we measure this?
	// TODO: Perhaps require use of a semaphore - is it worth it?
	if (commandBuffer->state == CBState::pending)
	{
		vtek_log_error("Command buffer cannot be reset from pending state!");
		return false;
	}
	if (commandBuffer->state == CBState::initial)
	{
		return true;
	}
	if (!commandBuffer->supportsReset)
	{
		vtek_log_error("Command buffer cannot be reset from a pool not supporting buffer reset!");
		return false;
	}

	// NOTE: `flags` may be the VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT flag, which
	// specifies that memory resources owned by the command buffer should be returned to the
	// parent command pool.
	VkCommandBufferResetFlags flags = 0;
	VkResult result = vkResetCommandBuffer(commandBuffer->vulkanHandle, flags);
	if (result != VK_SUCCESS)
	{
		vtek_log_error("Failed to reset command buffer!");
		return false;
	}

	return true;
}

bool vtek::command_buffer_free(vtek::CommandBuffer* commandBuffer, VkDevice device, VkCommandPool pool)
{
	if (commandBuffer->state == CBState::pending) // TODO: How do we measure this?
	{
		vtek_log_error("Command buffer cannot be freed from pending state!");
		return false;
	}

	const VkCommandBuffer buffers[] = { commandBuffer->vulkanHandle };
	vkFreeCommandBuffers(device, pool, 1, buffers);

	return true;
}

bool vtek::command_buffer_begin(vtek::CommandBuffer* commandBuffer)
{
	if (commandBuffer->state == CBState::pending) // TODO: How do we measure this?
	{
		vtek_log_error("Command buffer is in pending state and cannot begin recording!");
		return false;
	}
	if (commandBuffer->state == CBState::invalid) // TODO: How do we measure this?
	{
		vtek_log_error("Command buffer is in invalid state and must be reset before recording can begin!");
		return false;
	}
	if (commandBuffer->state == CBState::recording) // TODO: How do we measure this?
	{
		vtek_log_warn("Command buffer is already in recording state, so recorded commands may be lost!");
	}

	VkCommandBufferBeginInfo info{};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	info.pNext = nullptr;

	// VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT:
	//  - Specifies that each recording will only be submitted once, and then
	//    the command buffer will be reset and recorded again.
	// VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT:
	//  - Specifies that the secondary command buffer is considered to be entirely
	//    within a render pass. Ignored for primary command buffers.
	//  - `pInheritanceInfo.framebuffer` must be either VK_NULL_HANDLE or a valid
	//    VkFramebuffer that is compatible with `pInheritanceInfo.renderPass`.
	//  - pInheritanceInfo.renderPass must be either VK_NULL_HANDLE or a valid render pass.
	//  etc.
	// VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT:
	//  - Specifies that the command buffer can be resubmitted to a queue while in
	//    pending state, and recorded into multiple primary command buffers.
	info.flags = 0;
	info.pInheritanceInfo = nullptr;

	VkCommandBufferInheritanceInfo inheritanceInfo{};
	if (commandBuffer->isSecondary)
	{
		vtek_log_fatal("Secondary command buffer inheritance info is not implemented! Cannot begin recording.");
		return false;

		inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
		inheritanceInfo.pNext = nullptr;
		inheritanceInfo.renderPass = {}; // ??
		inheritanceInfo.subpass = 0; // ??
		inheritanceInfo.framebuffer = {}; // ??
		inheritanceInfo.occlusionQueryEnable = {}; // ??
		inheritanceInfo.queryFlags = 0; // ??
		inheritanceInfo.pipelineStatistics = {}; // ??
	}

	VkResult result = vkBeginCommandBuffer(commandBuffer->vulkanHandle, info);
	if (result != VK_SUCCESS)
	{
		vtek_log_error("Failed to begin command buffer recording!");
		return false;
	}

	return true;
}