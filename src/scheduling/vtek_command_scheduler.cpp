#include "vtek_vulkan.pch"
#include "vtek_command_scheduler.hpp"

//#include "vtek_command_pool.hpp"
#include "../../include/vtek/vtek_command_pool.hpp"

#include <mutex>


/* struct implementation */
struct vtek::CommandScheduler
{
	std::mutex mutex {};

	vtek::CommandPool* transferPool {nullptr};
	vtek::CommandPool* graphicsPool {nullptr};
	vtek::CommandPool* computePool {nullptr};
};


/* helper functions */
static void create_pools(vtek::CommandScheduler* scheduler, vtek::Device* device)
{
	vtek::Queue* graphicsQueue = vtek::device_get_graphics_queue(device);
	if (graphicsQueue != nullptr)
	{
		vtek::CommandPoolCreateInfo
		scheduler->graphicsPool =
			vtek::command_pool_create(graphicsInfo, device, graphicsQueue);
	}
}


/* interface */
vtek::CommandScheduler* vtek::command_scheduler_create(
	const vtek::CommandSchedulerInfo* info, vtek::Device* device)
{
	// Place at start to enable custom allocator
	auto scheduler = new vtek::CommandScheduler;


	scheduler->commandPool = vtek::command_pool_create(graphicsPoolInfo,device,const Queue* queue)


	// Figure out which queue families are created

	return scheduler;
}

vtek::CommandBuffer* vtek::command_scheduler_begin_singleuse_transfer()
{

}

void vtek::command_scheduler_end_singleuse_transfer(
	vtek::CommandBuffer* commandBuffer)
{

}
