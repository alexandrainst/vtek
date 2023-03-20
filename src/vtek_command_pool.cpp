// vtek
#include "vtek_command_pool.h"
#include "vtek_host_allocator.h"
#include "vtek_logging.h"


/* struct implementation */
struct vtek::CommandPool
{
	VkCommandPool vulkanHandle;
};


/* host allocator */
static vtek::HostAllocator<vtek::CommandPool> sAllocator("vtek_command_pool");


vtek::CommandPool* vtek::command_pool_create(
	const vtek::CommandPoolCreateInfo* info, const vtek::Device* device, const vtek::Queue* queue)
{
	// Allocate device
	vtek::CommandPool* commandPool = sAllocator.alloc();
	if (commandPool == nullptr)
	{
		vtek_log_error("Failed to allocate command pool!");
		return nullptr;
	}

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

void vtek::command_pool_destroy(const vtek::Device* device, vtek::CommandPool* commandPool)
{
	if (commandPool == nullptr) { return; }

	if (commandPool->vulkanHandle != VK_NULL_HANDLE)
	{
		VkDevice dev = vtek::device_get_handle(device);

		vkDestroyCommandPool(dev, commandPool->vulkanHandle, nullptr);
		commandPool->vulkanHandle = VK_NULL_HANDLE;
	}
}
