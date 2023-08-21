#pragma once

#include "vtek_object_handles.hpp"


namespace vtek
{
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

	struct CommandSchedulerInfo
	{
		// With a background thread running the command scheduler
		// all commands will be executed asynchronously, although the host
		// may explicitly synchronize as well.
		bool backgroundThread {false};
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
		CommandScheduler* scheduler, Device* device);

	bool command_scheduler_submit_transfer(
		CommandScheduler* scheduler, CommandBuffer* commandBuffer, Device* device);

	// Obtain a handle to the transfer queue used by the command scheduler
	// for issuing transfer operations.
	Queue* command_scheduler_get_transfer_queue(CommandScheduler* scheduler);
}
