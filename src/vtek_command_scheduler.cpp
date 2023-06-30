#include "vtek_vulkan.pch"
#include "vtek_command_scheduler.hpp"

#include "impl/vtek_queue_struct.hpp"
#include "vtek_command_buffer.hpp"
#include "vtek_command_pool.hpp"
#include "vtek_device.hpp"
#include "vtek_logging.hpp"
#include "vtek_queue.hpp"
#include "vtek_submit_info.hpp"

#include <mutex>
#include <vector>


/* struct implementation */
struct vtek::CommandScheduler
{
	std::mutex mutex {};

	vtek::CommandPool* transferPool {nullptr};
	vtek::CommandPool* graphicsPool {nullptr};
	vtek::CommandPool* computePool {nullptr};

	vtek::Queue* transferQueue {nullptr};
	vtek::Queue* graphicsQueue {nullptr};
	vtek::Queue* computeQueue {nullptr};
};



/* helper functions */
static bool create_pools(vtek::CommandScheduler* scheduler, vtek::Device* device)
{
	vtek::Queue* graphicsQueue = vtek::device_get_graphics_queue(device);
	if (graphicsQueue != nullptr)
	{
		vtek::CommandPoolInfo graphicsInfo{};
		graphicsInfo.allowIndividualBufferReset = true;
		graphicsInfo.hintRerecordOften = true;
		scheduler->graphicsPool =
			vtek::command_pool_create(&graphicsInfo, device, graphicsQueue);

		if (scheduler->graphicsPool == nullptr)
		{
			vtek_log_error(
				"vtek_command_scheduler.cpp: Failed to create graphics pool!");
			return false;
		}
		scheduler->graphicsQueue = graphicsQueue;
	}

	std::vector<vtek::Queue*> computeQueues =
		vtek::device_get_compute_queues(device);
	if (!computeQueues.empty())
	{
		vtek::CommandPoolInfo computeInfo{};
		computeInfo.allowIndividualBufferReset = true;
		computeInfo.hintRerecordOften = true;
		scheduler->computePool =
			vtek::command_pool_create(&computeInfo, device, computeQueues.back());

		if (scheduler->computePool == nullptr)
		{
			vtek_log_error(
				"vtek_command_scheduler.cpp: Failed to create compute pool!");
			return false;
		}
		scheduler->computeQueue = computeQueues.back();
	}

	// There must always be a transfer queue, and its support is implicitly
	// guaranteed for any queue family. Failure to support indicate either a
	// bug in vtek or a bug in the graphics driver.
	std::vector<vtek::Queue*> transferQueues =
		vtek::device_get_transfer_queues(device);
	if (transferQueues.size() == 0)
	{
		vtek_log_fatal(
			"vtek_command_scheduler.cpp: Failed to get transfer queues!");
		return false;
	}
	vtek::CommandPoolInfo transferInfo{};
	transferInfo.allowIndividualBufferReset = true;
	transferInfo.hintRerecordOften = true;
	scheduler->transferPool = vtek::command_pool_create(
		&transferInfo, device, transferQueues.back());

	if (scheduler->transferPool == nullptr)
	{
		vtek_log_error(
			"vtek_command_scheduler.cpp: Failed to create transfer pool!");
		return false;
	}
	scheduler->transferQueue = transferQueues.back();

	return true;
}



/* interface */
vtek::CommandScheduler* vtek::command_scheduler_create(
	const vtek::CommandSchedulerInfo* info, vtek::Device* device)
{
	// Place at start to enable custom allocator
	auto scheduler = new vtek::CommandScheduler;

	if (!create_pools(scheduler, device))
	{
		vtek_log_error("Failed to create all pools for command scheduler!");
		delete scheduler;
		return nullptr;
	}

	return scheduler;
}

void vtek::command_scheduler_destroy(
	vtek::CommandScheduler* scheduler, vtek::Device* device)
{
	// TODO: Mutex ?
	if (scheduler == nullptr) { return; }

	vtek::command_pool_destroy(scheduler->graphicsPool, device); // TODO: Valgrind complains about this!
	vtek::command_pool_destroy(scheduler->computePool, device);
	vtek::command_pool_destroy(scheduler->transferPool, device);

	delete scheduler;
}

vtek::CommandBuffer* vtek::command_scheduler_begin_transfer(
	vtek::CommandScheduler* scheduler, vtek::Device* device)
{
	vtek::CommandBuffer* buffer = vtek::command_pool_alloc_buffer(
		scheduler->transferPool, vtek::CommandBufferUsage::primary, device);
	if (buffer == nullptr)
	{
		vtek_log_error("Failed to allocate single-use transfer command buffer");
		return nullptr;
	}

	if (!vtek::command_buffer_begin(buffer))
	{
		vtek_log_error(
			"Failed to begin recording on single-use transfer command buffer!");
		vtek::command_pool_free_buffer(scheduler->transferPool, buffer, device);
		return nullptr;
	}

	return buffer;
}

bool vtek::command_scheduler_submit_transfer(
	vtek::CommandScheduler* scheduler, vtek::CommandBuffer* buffer)
{
	if (!vtek::command_buffer_end(buffer))
	{
		vtek_log_error(
			"Failed to end recording on single-use transfer command buffer!");
		return false;
	}

	// TODO: Wait for fence?
	// TODO: -- or force synchronous?
	vtek::SubmitInfo submitInfo{};

	if (!vtek::queue_submit(scheduler->transferQueue, buffer, &submitInfo))
	{
		vtek_log_error("Failed to submit single-use transfer command buffer!");
		return false;
	}

	// TODO: Right now, wait fence is not implemented, nor await or async, so we wait!
	vtek::queue_wait_idle(scheduler->transferQueue);

	return true;
}
