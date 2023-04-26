#pragma once

#include <vulkan/vulkan.h>
#include <memory>

#include "vtek_types.hpp"


namespace vtek
{
	struct CommandPoolCreateInfo
	{
		// Allow individual command buffers to be re-recorded individually.
		// Without this flag, only the entire pool may be reset at once.
		bool allowIndividualBufferReset {false};

		// If command buffers are rerecorded often this flag may be set to
		// hint the driver at a more suitable allocation behaviour.
		bool hintRerecordOften {false}; // TODO: Better name?
	};

	struct CommandPool; // opaque handle



	CommandPool* command_pool_create(
		const CommandPoolCreateInfo* info, const Device* device, const Queue* queue);

	void command_pool_destroy(CommandPool* commandPool, const Device* device);

	VkCommandPool command_pool_get_handle(CommandPool* commandPool);

	// The return value directly reflects the ´allowIndividualBufferReset´ flag
	// passed to the `CommandPoolCreateInfo` when the pool was created.
	// If it is false, command buffers allocated from this pool may still be
	// reset, but only by resetting them all at once!
	bool command_pool_allow_individual_reset(CommandPool* commandPool);
}