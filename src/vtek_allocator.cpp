#include "vtek_vulkan.pch"
#include "vtek_allocator.hpp"

#include "impl/vtek_vma_helpers.hpp" // Provides VMA include
#include "vtek_device.hpp"
#include "vtek_instance.hpp"
#include "vtek_logging.hpp"
#include "vtek_queue.hpp"
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
	if (allocator == nullptr) return;

	vmaDestroyAllocator(allocator->vmaHandle);
	allocator->vmaHandle = VK_NULL_HANDLE;

	delete allocator;
}

vtek::Allocator* vtek::allocator_create_default(
	vtek::Device* device, const vtek::Instance* instance)
{
	VkInstance inst = vtek::instance_get_handle(instance);
	VkDevice dev = vtek::device_get_handle(device);
	VkPhysicalDevice physDev = vtek::device_get_physical_handle(device);

	auto vv = device_get_vulkan_version(device);

	VmaVulkanFunctions vulkanFunctions{};
	vulkanFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
	vulkanFunctions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;

	VmaAllocatorCreateInfo createInfo{};
	createInfo.vulkanApiVersion = vv.apiVersion();
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
using IUFlag = vtek::ImageUsageFlag;
using ILayout = vtek::ImageLayout;

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

static VkImageUsageFlags get_image_usage_flags(vtek::EnumBitmask<IUFlag> mask)
{

	VkImageUsageFlags flags {0};

	if (mask.has_flag(IUFlag::transfer_src)) {
		flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	}
	if (mask.has_flag(IUFlag::transfer_dst)) {
		flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}
	if (mask.has_flag(IUFlag::sampled)) {
		flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
	}
	if (mask.has_flag(IUFlag::storage)) {
		flags |= VK_IMAGE_USAGE_STORAGE_BIT;
	}
	if (mask.has_flag(IUFlag::color_attachment)) {
		flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	}
	if (mask.has_flag(IUFlag::depth_stencil_attachment)) {
		flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	}
	if (mask.has_flag(IUFlag::transient_attachment)) {
		flags |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
	}
	if (mask.has_flag(IUFlag::input_attachment)) {
		flags |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
	}

	return flags;
}

// A staging buffer that will be filled via mapped pointer and then used as a source
// of transfer to the buffer can be created like this. It will likely end up in a
// memory type that is HOST_VISIBLE and HOST_COHERENT, but not HOST_CACHED (meaning
// uncached, write-combined), and not DEVICE_LOCAL (meaning system RAM).
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

void vtek::allocator_buffer_destroy(vtek::Buffer* buffer)
{
	VmaAllocator alloc = buffer->allocator->vmaHandle;

	vmaDestroyBuffer(alloc, buffer->vulkanHandle, buffer->vmaHandle);
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

bool vtek::allocator_image2d_create(
	vtek::Allocator* allocator, const vtek::Image2DInfo* info,
	vtek::Image2D* outImage)
{
	// 1) Fill `VkImageCreateInfo` struct
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.pNext = nullptr;
	imageInfo.flags = 0; // TODO: Optional `VkImageCreateFlagBits`
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent = { info->extent.width, info->extent.height, 1 };
	imageInfo.arrayLayers = 1;
	imageInfo.samples = vtek::get_multisample_count(info->multisampling);
	imageInfo.usage = get_image_usage_flags(info->usageFlags);

	if (info->format != vtek::Format::undefined) {
		imageInfo.format = vtek::get_format(info->format);
	}
	else if (!info->supportedFormat.is_valid()) {
		vtek_log_error("allocator_image2d_create(): {} -- {}",
		               "No valid image format was provided",
		               "cannot allocate image memory!");
		return false;
	}
	else {
		imageInfo.format = info->supportedFormat.get();
	}

	if (info->useMipmaps) {
		imageInfo.mipLevels = 4; // TODO: Perform proper calculation!
	}
	else {
		imageInfo.mipLevels = 1;
	}

	if (info->initialLayout == vtek::ImageInitialLayout::preinitialized &&
	    info->usageFlags.has_flag(vtek::ImageUsageFlag::transfer_dst))
	{
		imageInfo.tiling = VK_IMAGE_TILING_LINEAR;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
	}
	else
	{
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	}

	// Sharing mode, if the image is accessed by multiple queue families
	std::vector<uint32_t> queueIndices;
	if (info->sharingMode == vtek::ImageSharingMode::exclusive)
	{
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.queueFamilyIndexCount = 0;
		imageInfo.pQueueFamilyIndices = nullptr;
	}
	else
	{
		imageInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
		for (const auto* q : info->sharingQueues)
		{
			queueIndices.push_back(vtek::queue_get_family_index(q));
		}
		imageInfo.queueFamilyIndexCount = queueIndices.size();
		imageInfo.pQueueFamilyIndices = queueIndices.data();
	}

	// 2) Fill `VmaAllocationCreateinfo` struct
	VmaAllocationCreateInfo createInfo{};
	createinfo_devicelocal(&createInfo); // NOTE: Device-local preference assumed!
	if (info->requireDedicatedAllocation)
	{
		createInfo.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
	}

	// 3) Create allocation and image
	VkImage image;
	VmaAllocation allocation;
	vmaCreateImage(allocator->vmaHandle, &imageInfo, &createInfo, &image,
		&allocation, nullptr);
	if (image == VK_NULL_HANDLE || allocation == VK_NULL_HANDLE)
	{
		vtek_log_error("Failed to create 2D image with vma!");
		return false;
	}

	outImage->vulkanHandle = image;
	outImage->vmaHandle = allocation;
	outImage->extent = info->extent;
	outImage->format = imageInfo.format;
	outImage->allocator = allocator;

	return true;
}

void vtek::allocator_image2d_destroy(vtek::Image2D* image)
{
	VmaAllocator alloc = image->allocator->vmaHandle;

	vmaDestroyImage(alloc, image->vulkanHandle, image->vmaHandle);
}

// TODO: Image map and layout transition into optimal tiling !!!
