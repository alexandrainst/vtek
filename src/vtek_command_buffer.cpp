#include "vtek_vulkan.pch"
#include "vtek_command_buffer.hpp"

#include "impl/vtek_command_buffer_struct.hpp"
#include "vtek_command_pool.hpp"
#include "vtek_device.hpp"
#include "vtek_logging.hpp"

using CBState = vtek::CommandBufferStateType;



/* interface */
VkCommandBuffer vtek::command_buffer_get_handle(vtek::CommandBuffer* commandBuffer)
{
	return commandBuffer->vulkanHandle;
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
		vtek_log_error(
			"Command buffer is in invalid state and must be reset before recording can begin!");
		return false;
	}
	if (commandBuffer->state == CBState::recording) // TODO: How do we measure this?
	{
		vtek_log_warn(
			"Command buffer is already in recording state, so recorded commands may be lost!");
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
	// TODO: Add support for these flags?
	info.flags = 0;
	info.pInheritanceInfo = nullptr;

	VkCommandBufferInheritanceInfo inheritanceInfo{};
	if (commandBuffer->isSecondary)
	{
		vtek_log_fatal(
			"Secondary command buffer inheritance info is not implemented! Cannot begin recording.");
		return false;

		inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
		inheritanceInfo.pNext = nullptr;
		inheritanceInfo.renderPass = {}; // ??
		inheritanceInfo.subpass = 0; // ??
		inheritanceInfo.framebuffer = {}; // ??
		inheritanceInfo.occlusionQueryEnable = {}; // ??
		inheritanceInfo.queryFlags = 0; // ??
		inheritanceInfo.pipelineStatistics = {}; // ??

		inheritanceInfo = inheritanceInfo; // TODO: Just to eliminate compiler warning!
	}

	VkResult result = vkBeginCommandBuffer(commandBuffer->vulkanHandle, &info);
	if (result != VK_SUCCESS)
	{
		vtek_log_error("Failed to begin command buffer recording!");
		return false;
	}

	commandBuffer->state = CBState::recording;
	return true;
}

bool vtek::command_buffer_end(vtek::CommandBuffer* commandBuffer)
{
	if (commandBuffer->state != CBState::recording)
	{
		vtek_log_error(
			"Command buffer is not in recording state, so recording cannot end!");
		return false;
	}

	VkResult result = vkEndCommandBuffer(commandBuffer->vulkanHandle);
	if (result != VK_SUCCESS)
	{
		vtek_log_error("Failed to end command buffer recording!");
		return false;
	}

	commandBuffer->state = CBState::executable;
	return true;
}

bool vtek::command_buffer_is_recording(vtek::CommandBuffer* commandBuffer)
{
	return commandBuffer->state == CBState::recording;
}
