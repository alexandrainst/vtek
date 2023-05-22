#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#include "vk_mem_alloc.h"

/* struct implementation */
struct vtek::Allocator
{
	VmaAllocator vmaHandle {nullptr};
};
struct vtek::Buffer
{
	VkBuffer vulkanHandle {VK_NULL_HANDLE};
	VmaAllocation allocationHandle {nullptr};
};

vtek::Allocator* create_allocator(vtek::Device* device, vtek::Instance* instance)
{
	VkInstance inst = vtek::instance_get_handle(instance);
	VkDevice dev = vtek::device_get_handle(device);
	VkPhysicalDevice physDev = vtek::device_get_physical_handle(device);

	const vtek::VulkanVersion* vv = device_get_vulkan_version(device);

	VmaVulkanFunctions vulkanFunctions{};
	vulkanFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
	vulkanFunctions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;

	VmaAllocatorCreateInfo createInfo{};

#if defined(VK_API_VERSION_1_3)
	if (vv.minor >= 3) createInfo.vulkanApiVersion = VK_API_VERSION_1_3;
#elif defined(VK_API_VERSION_1_2)
	if (vv.minor == 2) createInfo.vulkanApiVersion = VK_API_VERSION_1_2;
#elif defined(VK_API_VERSION_1_1)
	if (vv.minor == 1) createInfo.vulkanApiVersion = VK_API_VERSION_1_1;
#else
	createInfo.vulkanApiVersion = VK_API_VERSION_1_0;
#endif

	createInfo.physicalDevice = physDev;
	createInfo.device = dev;
	createInfo.instance = inst;
	createInfo.pVulkanFunctions = &vulkanFunctions;

	VmaAllocator allocator;
	vmaCreateAllocator(&allocatorCreateInfo, &allocator);
}


vtek::Buffer* create_buffer(const vtek::BufferInfo* info, vtek::Allocator* allocator)
{
	VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	bufferInfo.size = 65536;
	bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_AUTO;

	VkBuffer buffer;
	VmaAllocation allocation;
	vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &buffer, &allocation, nullptr);
}

void destroy_allocator(vtek::Allocator* allocator)
{
	vmaDestroyAllocator(allocator->vmaHandle);
}

void destroy_buffer(vtek::Buffer* buffer, vtek::Allocator* allocator)
{
	vmaDestroyBuffer(allocator->vmaHandle, buffer->vulkanHandle, allocationHandle);
}
