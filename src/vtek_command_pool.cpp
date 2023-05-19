#include "vtek_command_pool.hpp"

#include "impl/vtek_host_allocator.hpp"
#include "vtek_device.hpp"
#include "vtek_logging.hpp"
#include "vtek_queue.hpp"


/* struct implementation */
struct vtek::CommandPool
{
	uint64_t id {VTEK_INVALID_ID};
	VkCommandPool vulkanHandle {VK_NULL_HANDLE};
	bool allowIndividualBufferReset {false};
};


/* host allocator */
static vtek::HostAllocator<vtek::CommandPool> sAllocator("vtek_command_pool");


/* interface */
vtek::CommandPool* vtek::command_pool_create(
	const vtek::CommandPoolCreateInfo* info, const vtek::Device* device, const vtek::Queue* queue)
{
	// Allocate device
	auto [id, commandPool] = sAllocator.alloc();
	if (commandPool == nullptr)
	{
		vtek_log_error("Failed to allocate command pool!");
		return nullptr;
	}
	commandPool->id = id;

	VkCommandPoolCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	uint32_t index = vtek::queue_get_family_index(queue);
	createInfo.queueFamilyIndex = index;

	// There are two flags, which can be set for command pools.
	// - VK_COMMAND_POOL_CREATE_TRANSIENT_BIT:
	//     Hint that command buffers are rerecorded with new commands
	//     very often (may change memory allocation behaviour).
	// - VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT:
	//     Allow command buffers to be rerecorded individually, without
	//     this flag they all have to be reset.
	createInfo.flags = 0;
	if (info->allowIndividualBufferReset)
	{
		createInfo.flags |= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		commandPool->allowIndividualBufferReset = true;
	}
	if (info->hintRerecordOften)
	{
		createInfo.flags |= VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
	}

	VkDevice dev = vtek::device_get_handle(device);
	VkResult result = vkCreateCommandPool(dev, &createInfo, nullptr, &commandPool->vulkanHandle);
	if (result != VK_SUCCESS)
	{
		vtek_log_error("Failed to create command pool!");
		return nullptr;
	}

	return commandPool;
}

void vtek::command_pool_destroy(vtek::CommandPool* commandPool, const vtek::Device* device)
{
	if (commandPool == nullptr) { return; }

	if (commandPool->vulkanHandle != VK_NULL_HANDLE)
	{
		VkDevice dev = vtek::device_get_handle(device);

		vkDestroyCommandPool(dev, commandPool->vulkanHandle, nullptr);
		commandPool->vulkanHandle = VK_NULL_HANDLE;
	}

	sAllocator.free(commandPool->id);
	commandPool->id = VTEK_INVALID_ID;
}

VkCommandPool vtek::command_pool_get_handle(vtek::CommandPool* commandPool)
{
	return commandPool->vulkanHandle;
}

bool vtek::command_pool_allow_individual_reset(vtek::CommandPool* commandPool)
{
	return commandPool->allowIndividualBufferReset;
}
