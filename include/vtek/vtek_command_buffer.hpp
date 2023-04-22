#pragma once

#include <vector>
#include <vulkan/vulkan.h>

#include "vtek_types.hpp"


namespace vtek
{
	// State for a command buffer. See:
	// https://registry.khronos.org/vulkan/specs/1.2-extensions/html/vkspec.html#commandbuffers-lifecycle
	enum class CommandBufferStateType
	{
		// A command buffer is not allocated, or it was explicitly deallocated.
		// TODO: Do we need this safety measure?
		not_allocated,

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

	struct CommandBufferCreateInfo
	{
		bool isSecondary {false};
	};


	// TODO: Rename to `command_buffer_alloc` ?
	CommandBuffer* command_buffer_create(
		const CommandBufferCreateInfo* info, CommandPool* pool, Device* device);

	std::vector<CommandBuffer*> command_buffer_create(
		const CommandBufferCreateInfo* info, uint32_t createCount, CommandPool* pool, Device* device);

	// TODO: Rename to `command_buffer_free` ?
	void command_buffer_destroy(CommandBuffer* commandBuffer, Device* device);
	void command_buffer_destroy(std::vector<CommandBuffer*>& commandBuffers, Device* device);

	VkCommandBuffer command_buffer_get_handle(CommandBuffer* commandBuffer);
	VkCommandPool command_buffer_get_pool_handle(CommandBuffer* commandBuffer);

	// TODO: Perhaps we want to pack the details away and instead provide this interface:
	bool command_buffer_reset(CommandBuffer* commandBuffer);
	bool command_buffer_free(CommandBuffer* commandBuffer, VkDevice device, VkCommandPool pool);
	bool command_buffer_begin(CommandBuffer* commandBuffer);
	bool command_buffer_end(CommandBuffer* commandBuffer);

	// TODO: Single-use command buffers! Considerations:
	// - waiting for buffer to finish execution?
	// - after finished execution, how to free the buffer?
	// - do we create a fence for this particular buffer and signal that when done?
	// - cleanup routine inside vtek to handle finished buffers?
	// - should these buffers be created from a dedicated pool?
	// - if dedicated cleanup thread inside vtek, then we need a dedicated pool for these buffers!
	//
	// Perhaps better to just expose a cleanup function that client applications may call
	// once a while, and let clients choose how to handle threading.
	// Then just inform them that this cleanup routine should be called!
}
