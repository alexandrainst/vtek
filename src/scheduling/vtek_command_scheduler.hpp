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

	struct CommandScheduler; // opaque handle

	CommandScheduler* command_scheduler_create(
		const CommandSchedulerInfo* info, Device* device);

	CommandBuffer* command_scheduler_begin_singleuse_transfer();
	void command_scheduler_end_singleuse_transfer(CommandBuffer* commandBuffer);
}
