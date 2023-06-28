#pragma once

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

#include "vtek_vulkan_handles.hpp"


namespace vtek
{
	struct CommandPoolInfo
	{
		// Allow individual command buffers to be re-recorded individually.
		// Without this flag, only the entire pool may be reset at once.
		bool allowIndividualBufferReset {false};

		// If command buffers are rerecorded often this flag may be set to
		// hint the driver at a more suitable allocation behaviour.
		bool hintRerecordOften {false}; // TODO: Better name?
	};


	CommandPool* command_pool_create(
		const CommandPoolInfo* info, const Device* device, const Queue* queue);

	void command_pool_destroy(CommandPool* commandPool, const Device* device);

	VkCommandPool command_pool_get_handle(CommandPool* commandPool);

	// The return value directly reflects the ´allowIndividualBufferReset´ flag
	// passed to the `CommandPoolInfo` when the pool was created.
	// If it is false, command buffers allocated from this pool may still be
	// reset, but only by resetting them all at once!
	bool command_pool_allow_individual_reset(CommandPool* commandPool);

	// =========================== //
	// === Considering New API === //
	// =========================== //

	// Reset the pool which resets all buffers.
	// TODO: Does it also free all allocated command buffers, or only reset them?
	// TODO: Should we check that no allocated command buffers are pending execution?
	void command_pool_reset(CommandPool* commandPool);

	enum class CommandBufferUsage
	{
		primary,
		secondary
	};

	CommandBuffer* command_pool_alloc_buffer(
		CommandPool* pool, CommandBufferUsage usage, Device* device);

	std::vector<CommandBuffer*> command_pool_alloc_buffers(
		CommandPool* pool, CommandBufferUsage usage,
		uint32_t numBuffers, Device* device);

	void command_pool_free_buffer(
		CommandPool* pool, CommandBuffer* buffer, Device* device);

	void command_pool_free_buffers(
		CommandPool* pool, std::vector<CommandBuffer*> buffers, Device* device);

	bool command_pool_reset_buffer(CommandPool* pool, CommandBuffer* buffer);
}
