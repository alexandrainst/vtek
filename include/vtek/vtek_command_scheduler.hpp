#pragma once

#include "vtek_vulkan_handles.hpp"


namespace vtek
{
	struct CommandSchedulerInfo
	{
		// With a background thread running the command scheduler
		// all commands will be executed asynchronously, although the host
		// may explicitly synchronize as well.
		bool backgroundThread {true};
	};


	// Create a command scheduler, an object which issues single-use command
	// buffers, e.g. for transfer operations.
	CommandScheduler* command_scheduler_create(
		const CommandSchedulerInfo* info, Device* device);

	void command_scheduler_destroy(CommandScheduler* scheduler, Device* device);

	// Create and submit a single-use transfer command buffer.
	// The `begin` function creates a command buffer from an internally
	// managed, thread-safe command pool, and begins recording into it.
	// The `end` function ends the command buffer recording and submits it
	// for execution onto an internally referenced transfer queue.
	// TODO: Create dedicated command buffer type to distinguish ?
	// TODO: E.g. `SingleUseCommandBuffer` ?
	CommandBuffer* command_scheduler_begin_transfer(
		CommandScheduler* scheduler);

	void command_scheduler_submit_transfer(
		CommandScheduler* scheduler, CommandBuffer* commandBuffer);
}
