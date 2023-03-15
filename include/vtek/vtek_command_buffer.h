#pragma once

namespace vtek
{
	// State for a command buffer. See:
	// https://registry.khronos.org/vulkan/specs/1.2-extensions/html/vkspec.html#commandbuffers-lifecycle
	enum class CommandBufferStateType
	{
		// When a command buffer has been allocated.
		// Some commands can reset a command buffer back to this state from executable, recording, or invalid.
		// Command buffers in initial state can only be moved to recording state, or freed.
		initial,

		// "vkBeginCommandBuffer" sets the state to recording, which enables vkCmd* commands to be issued.
		recording,

		// "vkEndCommandBuffer" ends recording and sets state to executable, which means that a command
		// buffer may be submitted to a queue, reset, or recorded to another command buffer (secondary).
		executable,

		// "vkQueueSubmit" sets state from executable to pending.
		// A command buffer in pending state *must not* be modified by an application in any way, as the
		// device may be processing commands recorded to it.
		// Once execution completes, the command buffer reverts back to executable state, or invalid if
		// it was recorded with "one_time_submit" flag set.
		pending,

		// Some operations, such as modifying or deleting a resource used in a recorded command, will
		// transition a command buffer to invalid state.
		// Command buffers in invalid state may only be reset or freed.
		invalid
	};

	struct CommandBuffer; // opaque handle

	CommandBuffer* command_buffer_create();
	void command_buffer_destroy(CommandBuffer* commandBuffer);

	// TODO: Perhaps we want to pack the details away and instead provide this interface:
	bool command_buffer_reset(CommandBuffer* commandBuffer);
	bool command_buffer_free(CommandBuffer* commandBuffer, VkDevice device, VkCommandPool pool);
	bool command_buffer_begin(CommandBuffer* commandBuffer);
	bool command_buffer_end(CommandBuffer* commandBuffer);
	bool command_buffer_submit(CommandBuffer* commandBuffer, VkQueue queue);
}
