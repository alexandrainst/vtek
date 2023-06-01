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


void specify_memory_requirements()
{
	// Fill usage
	VmaAllocationCreateInfo allocInfo{};
	allocInfo.usage = ... ; // use values of enum VmaMemoryUsage
	// Since version 3 of vma library, it's recommended to use VM_MEMORY_USAGE_AUTO
	// to let it select best memory type for the resource automatically.
}

// If you want to create a uniform buffer that will be filled using transfer only
// once or infrequently, and then used for rendering every frame as a uniform buffer,
// you can do it using the following code. The buffer will _most likely_ end up in
// a memory type with VK_MEMORY_PROPERTIES_DEVICE_LOCAL_BIT to be fast to access by
// the GPU device.
void create_uniform_buffer_static_draw(vtek::Allocator* a)
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = 65536;
	bufferInfo.usage
		= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
		| VK_BUFFER_USAGE_TRANSFER_DST_BIT;

	VmaAllocationCreateInfo allocInfo{};
	allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
	// Other potential usages:
	// VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE
	// VMA_MEMORY_USAGE_AUTO_PREFER_HOST
	// This can be used for memory systems with discrete graphics card.

	// When using VMA_MEMORY_USAGE_AUTO* while wanting to map the allocated memory,
	// then specify on of the following host access flags:
	// VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
	// VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT
	// This will help vma decide about preferred memory type to _ensure_ it has
	// VM_MEMORY_PROPERTY_HOST_VISIBLE_BIT, so it can be mapped.

	VkBuffer buffer;
	VmaAllocation allocation;
	vmaCreateBuffer(a->vmaHandle, &bufferInfo, &allocInfo, &buffer, &allocation, nullptr);
	if (buffer == VK_NULL_HANDLE)
	{

	}
	if (allocation == nullptr)
	{

	}

	// OKAY: Success, now return a uniform buffer handle!
}

// A staging buffer that will be filled via mapped pointer and then used as a source
// of transfer to the buffer can be created like this. It will likely end up in a
// memory type that is HOST_VISIBLE and HOST_COHERENT, but not HOST_CACHED (meaning
// uncached, write-combined), and not DEVICE_LOCAL (meaning system RAM).
void create_host_visible_staging_buffer(vtek::Allocator* a)
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = 65536;
	bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

	VmaAllocationCreateInfo allocInfo{};
	allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
	allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

	VkBuffer buffer;
	VmaAllocation allocation;
	vmaCreateBuffer(a->vmaHandle, &bufferInfo, &allocInfo, &buffer, &allocation, nullptr);
}

// Create a buffer with required flag mapped on host (HOST_VISIBLE), but then also
// _preferred_ to be HOST_COHERENT and HOST_CACHED:
void create_host_visible_and_more(vtek::Allocator* a)
{
	VmaAllocationCreateInfo allocInfo{};
	allocInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
	allocInfo.preferredFlags
		= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		| VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
	allocInfo.flags
		= VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT
		| VMA_ALLOCATION_CREATE_MAPPED_BIT;

	VkBuffer buffer;
	VmaAllocation allocation;
	vmaCreateBuffer(a->vmaHandle, &bufferInfo, &allocInfo, &buffer, &allocation, nullptr);
}


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

	// This function will create a buffer, allocate memory for it, and bind them together.
	// This is the recommended way to use vma library.
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

bool vtek::buffer_write_data(Buffer* buffer, void* data, uint64_t size, Device* device)
{

}
