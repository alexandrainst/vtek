#include "vtek_allocator.hpp"

#include "impl/vtek_vma_helpers.hpp"
#include "vtek_device.hpp"
#include "vtek_instance.hpp"
#include "vtek_logging.hpp"
#include "vtek_vulkan_version.hpp"


/* struct implementation */
struct vtek::Allocator
{
	VmaAllocator vmaHandle {nullptr};
};


/* PUBLIC interface */
vtek::Allocator* vtek::allocator_create(
	vtek::Device* device, const vtek::Instance* instance,
	const vtek::AllocatorInfo* info)
{
	vtek_log_error("vtek::allocator_create: Not implemented!");
	return nullptr;
}

void vtek::allocator_destroy(Allocator* allocator)
{
	vtek_log_error("vtek::allocator_destroy: Not implemented!");
}

vtek::Allocator* vtek::allocator_create_default(
	vtek::Device* device, const vtek::Instance* instance)
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

	auto allocator = new vtek::Allocator;
	vmaCreateAllocator(&createInfo, &allocator->vmaHandle);
	if (allocator->vmaHandle == VK_NULL_HANDLE)
	{
		vtek_log_error("Failed to create default (vma) allocator!");
		delete allocator;
		return nullptr;
	}

	return allocator;
}


/* helper functions */
using BUFlag = vtek::BufferUsageFlag;

static VkBufferUsageFlags get_buffer_usage_flags(vtek::EnumBitmask<BUFlag> mask)
{
	VkBufferUsageFlags flags {0};

	if (mask.has_flag(BUFlag::transfer_src)) {
		flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	}
	if (mask.has_flag(BUFlag::transfer_dst)) {
		flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	}
	if (mask.has_flag(BUFlag::uniform_texel_buffer)) {
		flags |= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
	}
	if (mask.has_flag(BUFlag::storage_texel_buffer)) {
		flags |= VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
	}
	if (mask.has_flag(BUFlag::uniform_buffer)) {
		flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	}
	if (mask.has_flag(BUFlag::storage_buffer)) {
		flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	}
	if (mask.has_flag(BUFlag::index_buffer)) {
		flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	}
	if (mask.has_flag(BUFlag::vertex_buffer)) {
		flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	}
	if (mask.has_flag(BUFlag::indirect_buffer)) {
		flags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
	}
	if (mask.has_flag(BUFlag::shader_device_address)) {
		flags |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
	}

	return flags;
}

static void createinfo_stagingbuffer(VmaAllocationCreateInfo* info)
{
	// When using VMA_MEMORY_USAGE_AUTO* while wanting to map the allocated
	// memory, then specify one of the following host access flags:
	// VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
	// VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT
	// This will help vma decide about preferred memory type to _ensure_ it has
	// VM_MEMORY_PROPERTY_HOST_VISIBLE_BIT, so it can be mapped.

	info->usage = VMA_MEMORY_USAGE_AUTO;
	info->flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
	info->preferredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT; // TODO: ?
}

static void createinfo_devicelocal(VmaAllocationCreateInfo* info)
{
	info->usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
	info->preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
}



/* INTERNAL interface */
bool vtek::allocator_buffer_create(
	vtek::Allocator* allocator, const vtek::BufferInfo* info,
	vtek::Buffer* outBuffer)
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = info->size; // TODO: Bump up to power of two?
	bufferInfo.usage = get_buffer_usage_flags(info->usageFlags);

	VmaAllocationCreateInfo createInfo{};
	if (info->requireHostVisibleStorage) {
		createinfo_stagingbuffer(&createInfo);
	}
	else {
		createinfo_devicelocal(&createInfo);
	}

	if (info->requireDedicatedAllocation)
	{
		createInfo.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
	}

	VkBuffer buffer;
	VmaAllocation allocation;
	vmaCreateBuffer(allocator->vmaHandle, &bufferInfo, &createInfo, &buffer,
		&allocation, nullptr);
	if (buffer == VK_NULL_HANDLE || allocation == VK_NULL_HANDLE)
	{
		// TODO: Leaks ?
		vtek_log_error("Failed to create buffer with vma!");
		return false;
	}

	VkMemoryPropertyFlags memFlags{0};
	vmaGetAllocationMemoryProperties(allocator->vmaHandle, allocation, &memFlags);
	vtek::EnumBitmask<vtek::MemoryProperty> mask{};

	if (memFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
		mask.add_flag(vtek::MemoryProperty::device_local);
	}
	if (memFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
		mask.add_flag(vtek::MemoryProperty::host_visible);
	}
	if (memFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {
		mask.add_flag(vtek::MemoryProperty::host_coherent);
	}
	if (memFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) {
		mask.add_flag(vtek::MemoryProperty::host_cached);
	}
	if (memFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) {
		mask.add_flag(vtek::MemoryProperty::lazily_allocated);
	}
#if defined(VK_API_VERSION_1_1)
	if (memFlags & VK_MEMORY_PROPERTY_PROTECTED_BIT) {
		mask.add_flag(vtek::MemoryProperty::memory_protected);
	}
#endif

	VmaAllocationInfo allocInfo{};
	vmaGetAllocationInfo(allocator->vmaHandle, allocation, &allocInfo);

	outBuffer->vulkanHandle = buffer;
	outBuffer->vmaHandle = allocation;
	outBuffer->size = allocInfo.size;
	outBuffer->allocator = allocator;
	outBuffer->memoryProperties = mask;

	return true;
}

void vtek::allocator_buffer_destroy(
	vtek::Allocator* allocator, vtek::Buffer* buffer)
{
	vtek_log_error("vtek::allocator_buffer_destroy: Not implemented!");
}

void* vtek::allocator_buffer_map(vtek::Buffer* buffer)
{
	VmaAllocator alloc = buffer->allocator->vmaHandle;

	void* mappedData;
	vmaMapMemory(alloc, buffer->vmaHandle, &mappedData);
	return mappedData;
}

void vtek::allocator_buffer_unmap(vtek::Buffer* buffer)
{
	VmaAllocator alloc = buffer->allocator->vmaHandle;

	vmaUnmapMemory(alloc, buffer->vmaHandle);
}

void vtek::allocator_buffer_flush(
	vtek::Buffer* buffer, const vtek::BufferRegion* region)
{
	VmaAllocator alloc = buffer->allocator->vmaHandle;

	VkResult result = vmaFlushAllocation(
		alloc, buffer->vmaHandle, region->offset, region->size);
	if (result != VK_SUCCESS)
	{
		vtek_log_error("Failed to flush buffer from allocator -- {}",
		               "data will likely not be visible!");
	}
}




/*
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


vtek::Allocator* allocator_create(vtek::Device* device, vtek::Instance* instance)
{
	vtek_log_error("vtek::allocator_create: Not implemented!");
	return nullptr;
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
	vmaDestroyBuffer(allocator->vmaHandle, buffer->vulkanHandle, buffer->allocation);
}

bool vtek::buffer_write_data(Buffer* buffer, void* data, uint64_t size, Device* device)
{

}

bool vtek::buffer_read_data(Buffer* buffer, std::vector<char>& dest, Device* device)
{
	if (buffer->memoryProperties.has_flag(MemoryProperty::host_visible))
	{
		// map/unmap
		void* mappedData;
		vmaMapMemory(allocator, buffer->allocation, &mappedData);
		const size_t bufferSize = buffer->size; // TODO: Maybe vma knows?

		dest.resize(bufferSize);
		memcpy(dest.data(), mappedData, bufferSize);

		vmaUnmapMemory(buffer->allocator, buffer->allocation);

		// potentially flush if not HOST_COHERENT

	}
}
*/
