#include "vtek_buffer.hpp"

#include "vtek_logging.hpp"

// VMA library
#include "impl/vtek_vma_helpers.hpp"


/* internal helper types */
// TODO: Do we want this?
enum class MemoryProperty : uint32_t
{
	device_local     = 0x0001U,
	host_visible     = 0x0002U,
	host_coherent    = 0x0004U,
	host_cached      = 0x0008U,
	lazily_allocated = 0x0010U,
	memory_protected = 0x0020U
};


/* VMA library */
#include "impl/vtek_init.hpp"
bool vtek::initialize_vma_allocator(const Instance* instance, const Device* device)
{
	VkInstance inst = vtek::instance_get_handle(instance);
	VkDevice dev = vtek::device_get_handle(device);
	VkPhysicalDevice physDev = vtek::device_get_physical_handle(device);

	const vtek::VulkanVersion* vv = device_get_vulkan_version(device);

	VmaVulkanFunctions vulkanFunctions{};
	vulkanFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
	vulkanFunctions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;

	VmaAllocatorCreateInfo createInfo{};
	createInfo.vulkanApiVersion = vv->apiVersion();
	createInfo.physicalDevice = physDev;
	createInfo.device = dev;
	createInfo.instance = inst;
	createInfo.pVulkanFunctions = &vulkanFunctions;

	VmaAllocator allocator;
	vmaCreateAllocator(&allocatorCreateInfo, &allocator);
}

void vtek::terminate_vma_allocator()
{

}






/* interface */
vtek::Buffer* vtek::buffer_create(
	const vtek::BufferInfo* info, vtek::Device* device)
{
	vtek::Allocator* allocator = vtek::device_get_allocator(device);
	VkBuffer handle = allocator_create_buffer(allocator, info);
	if (handle == VK_NULL_HANDLE)
	{
		vtek_log_error("Failed to create buffer!");
		return nullptr;
	}

	auto buffer = new vtek::Buffer;
	buffer->vulkanHandle = handle;
	buffer->allocator = allocator;

	return buffer;
}

void vtek::buffer_destroy(vtek::Buffer* buffer)
{
	vtek::allocator_buffer_destroy(buffer);
}



// ===================== //
// === Cache control === //
// ===================== //

// Any memory type that doesn't have HOST_COHERENT flag needs manual cache constrol:
// - vkInvalidateMappedMemoryRanges before read on CPU.
// - vkFlushMappedMemoryRanges after write on CPU.
