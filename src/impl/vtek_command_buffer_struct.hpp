// Internal header file, do not include.

#pragma once

// TODO: No longer use sAllocator ?
//#include "vtek_host_allocator.hpp"


namespace vtek
{
	// TODO: If this works, implement it somewhere else, and rewrite all allocators and structs!
	// struct OpaqueHandle
	// {
	// public:
	// 	uint64_t id {VTEK_INVALID_ID}; // Implemented in impl/vtek_host_allocator.h

	// 	inline OpaqueHandle() { id = global_id++; }
	// 	inline virtual ~OpaqueHandle() {}
	// private:
	// 	static inline uint64_t global_id = 0;
	// };


	// TODO: Do we _really_ need this? Can't Vulkan check it for us, and rid us of this overhead?!
	// State for a command buffer. See:
	// https://registry.khronos.org/vulkan/specs/1.2-extensions/html/vkspec.html#commandbuffers-lifecycle
	enum class CommandBufferStateType
	{
		// REVIEW: Travis also uses `in_render_pass`

		// A command buffer is not allocated, or it was explicitly deallocated.
		// TODO: Do we need this safety measure?
		not_allocated,

		// When a command buffer has been allocated. Some commands can reset a
		// command buffer back to this state from executable, recording, or
		// invalid. Command buffers in initial state can only be moved to
		// recording state, or freed.
		initial,

		// "vkBeginCommandBuffer" sets the state to recording, which enables
		// vkCmd* commands to be issued.
		recording,

		// "vkEndCommandBuffer" ends recording and sets state to executable,
		// which means that a command buffer may be submitted to a queue, reset,
		// or recorded to another command buffer (secondary).
		executable,

		// "vkQueueSubmit" sets state from executable to pending. A command
		// buffer in pending state *must not* be modified by an application in
		// any way, as the device may be processing commands recorded to it.
		// Once execution completes, the command buffer reverts back to
		// executable state, or invalid if it was recorded with
		// "one_time_submit" flag set.
		pending,

		// Some operations, such as modifying or deleting a resource used in a
		// recorded command, will transition a command buffer to invalid state.
		// Command buffers in invalid state may only be reset or freed.
		invalid
	};


	struct CommandBuffer
	{
		VkCommandBuffer vulkanHandle {VK_NULL_HANDLE};
		VkCommandPool poolHandle {VK_NULL_HANDLE}; // TODO: We might not need this with new API!
		CommandBufferStateType state {CommandBufferStateType::invalid};

		// If command buffer was created from a pool that was created with the
		// VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT flag. Otherwise the
		// command buffer cannot be reset.
		bool supportsReset {false};

		// Secondary command buffers may be recorded inside primary command buffers.
		bool isSecondary {false};
	};
}
