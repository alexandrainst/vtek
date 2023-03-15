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
		VTEK_LOG_ERROR("Failed to allocate command pool!");
		return nullptr;
	}

	VkCommandPoolCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	uint32_t index = vtek::queue_get_family_index(queue);
	std::cout << "graphics index: " << index << '\n';
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
		VTEK_LOG_ERROR("Failed to create command pool!");
		return nullptr;
	}

	return commandPool;
}

void vtek::command_pool_destroy(vtek::CommandPool* commandPool)
{

}
