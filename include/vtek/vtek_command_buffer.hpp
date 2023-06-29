#pragma once

#include <vector>
#include <vulkan/vulkan.h>

#include "vtek_vulkan_handles.hpp"


namespace vtek
{
	VkCommandBuffer command_buffer_get_handle(CommandBuffer* commandBuffer);

	bool command_buffer_begin(CommandBuffer* commandBuffer);
	bool command_buffer_end(CommandBuffer* commandBuffer);
	bool command_buffer_reset(CommandBuffer* commandBuffer);

	// TODO: Command wrappers here?
}
