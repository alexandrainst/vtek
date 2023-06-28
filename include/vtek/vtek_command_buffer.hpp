#pragma once

#include <vector>
#include <vulkan/vulkan.h>

#include "vtek_vulkan_handles.hpp"


namespace vtek
{
	struct CommandBufferCreateInfo // TODO: Delete anyways 'cause bad name.
	{
		bool isSecondary {false};
	};


	// TODO: Rename to `command_buffer_alloc` ?
	CommandBuffer* command_buffer_create(
		const CommandBufferCreateInfo* info, CommandPool* pool, Device* device);

	std::vector<CommandBuffer*> command_buffer_create(
		const CommandBufferCreateInfo* info, uint32_t createCount,
		CommandPool* pool, Device* device);

	// TODO: Rename to `command_buffer_free` ?
	void command_buffer_destroy(CommandBuffer* commandBuffer, Device* device);
	void command_buffer_destroy(
		std::vector<CommandBuffer*>& commandBuffers, Device* device);

	VkCommandBuffer command_buffer_get_handle(CommandBuffer* commandBuffer);
	VkCommandPool command_buffer_get_pool_handle(CommandBuffer* commandBuffer);

	bool command_buffer_begin(CommandBuffer* commandBuffer);
	bool command_buffer_end(CommandBuffer* commandBuffer);
	bool command_buffer_reset(CommandBuffer* commandBuffer);

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
