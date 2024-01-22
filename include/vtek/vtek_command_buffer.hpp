#pragma once

#include <vector>
#include <vulkan/vulkan.h>

#include "vtek_object_handles.hpp"


namespace vtek
{
	VkCommandBuffer command_buffer_get_handle(CommandBuffer* commandBuffer);

	struct CommandBufferBeginInfo
	{
		// Specifies that the recording will only be submitted once, and then
		// the command buffer will be reset and recorded agin.
		// NOTE: In order for this to have effect, the command pool from which
		// this command buffer was allocated must either have been created with
		// the `allowIndividualBufferReset` flag, or all allocated command
		// buffers must be reset at the same time.
		bool oneTimeSubmit {false};

		// Specifies that a secondary command buffer is considered to be
		// entirely within a render pass. Ignored for primary command buffers.
		bool renderPassContinue {false};

		// The command buffer may be resubmitted to a queue while in pending
		// state, and recorded into multiple primary command buffers.
		bool simultaneousUse {false};

		// TODO: Possibility to fill in inheritance info?
	};

	bool command_buffer_begin(
		CommandBuffer* commandBuffer, const CommandBufferBeginInfo* beginInfo);
	bool command_buffer_end(CommandBuffer* commandBuffer);

	bool command_buffer_is_recording(CommandBuffer* commandBuffer);
}
