#include "vtek_vulkan.pch"
#include "vtek_command_pool.hpp"

#include "impl/vtek_command_buffer_struct.hpp"
#include "vtek_device.hpp"
#include "vtek_logging.hpp"
#include "vtek_queue.hpp"

using CBState = vtek::CommandBufferStateType;


/* struct implementation */
struct vtek::CommandPool
{
	VkCommandPool vulkanHandle {VK_NULL_HANDLE};
	bool allowIndividualBufferReset {false};
};



/* interface */
vtek::CommandPool* vtek::command_pool_create(
	const vtek::CommandPoolInfo* info, const vtek::Device* device,
	const vtek::Queue* queue)
{
	// Allocate device
	auto commandPool = new vtek::CommandPool;

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
	VkResult result = vkCreateCommandPool(
		dev, &createInfo, nullptr, &commandPool->vulkanHandle);
	if (result != VK_SUCCESS)
	{
		vtek_log_error("Failed to create command pool!");
		delete commandPool;
		return nullptr;
	}

	return commandPool;
}

void vtek::command_pool_destroy(
	vtek::CommandPool* commandPool, const vtek::Device* device)
{
	if (commandPool == nullptr) { return; }

	if (commandPool->vulkanHandle != VK_NULL_HANDLE)
	{
		VkDevice dev = vtek::device_get_handle(device);

		vkDestroyCommandPool(dev, commandPool->vulkanHandle, nullptr);
		commandPool->vulkanHandle = VK_NULL_HANDLE;
	}

	delete commandPool;
}

VkCommandPool vtek::command_pool_get_handle(vtek::CommandPool* commandPool)
{
	return commandPool->vulkanHandle;
}

bool vtek::command_pool_allow_individual_reset(vtek::CommandPool* commandPool)
{
	return commandPool->allowIndividualBufferReset;
}


// =========================== //
// === Considering New API === //
// =========================== //

vtek::CommandBuffer* vtek::command_pool_alloc_buffer(
	vtek::CommandPool* pool, vtek::CommandBufferUsage usage,
	vtek::Device* device)
{
	// Placed at beginning to enable custom allocator
	auto commandBuffer = new vtek::CommandBuffer();

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.pNext = nullptr;
	allocInfo.commandPool = pool->vulkanHandle;
	allocInfo.level =
		(usage == vtek::CommandBufferUsage::primary)
		? VK_COMMAND_BUFFER_LEVEL_PRIMARY
		: VK_COMMAND_BUFFER_LEVEL_SECONDARY;
	allocInfo.commandBufferCount = 1;

	VkResult allocResult = vkAllocateCommandBuffers(
		vtek::device_get_handle(device), &allocInfo, &commandBuffer->vulkanHandle);
	if (allocResult != VK_SUCCESS)
	{
		vtek_log_error("Failed to allocate command buffer!");
		delete commandBuffer;
		return nullptr;
	}

	commandBuffer->isSecondary = (usage == vtek::CommandBufferUsage::secondary);
	commandBuffer->state = CBState::initial;
	return commandBuffer;
}

std::vector<vtek::CommandBuffer*> vtek::command_pool_alloc_buffers(
	vtek::CommandPool* pool, vtek::CommandBufferUsage usage,
	uint32_t numBuffers, vtek::Device* device)
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.pNext = nullptr;
	allocInfo.commandPool = pool->vulkanHandle;
	allocInfo.level =
		(usage == vtek::CommandBufferUsage::primary)
		? VK_COMMAND_BUFFER_LEVEL_PRIMARY
		: VK_COMMAND_BUFFER_LEVEL_SECONDARY;
	allocInfo.commandBufferCount = numBuffers;

	std::vector<VkCommandBuffer> vulkanHandles(numBuffers, VK_NULL_HANDLE);

	VkResult allocResult = vkAllocateCommandBuffers(
		vtek::device_get_handle(device), &allocInfo, vulkanHandles.data());
	if (allocResult != VK_SUCCESS)
	{
		vtek_log_error("Failed to allocate command buffer!");
		return {};
	}

	bool reset = pool->allowIndividualBufferReset;
	bool secondary = (usage == vtek::CommandBufferUsage::secondary);

	// NEXT: Allocate the wrapper objects
	std::vector<vtek::CommandBuffer*> commandBuffers;

	for (uint32_t i = 0; i < numBuffers; i++)
	{
		vtek::CommandBuffer* buffer = new vtek::CommandBuffer;
		buffer->vulkanHandle = vulkanHandles[i];
		buffer->supportsReset = reset;
		buffer->isSecondary = secondary;
		buffer->state = CBState::initial;

		commandBuffers.emplace_back(buffer);
	}

	return commandBuffers;
}

void vtek::command_pool_free_buffer(
	vtek::CommandPool* pool, vtek::CommandBuffer* buffer, vtek::Device* device)
{
	if (buffer->state == CBState::pending) // TODO: How do we measure this?
	{
		vtek_log_error("Command buffer cannot be freed from pending state!");
		return;
	}

	VkDevice dev = vtek::device_get_handle(device);
	const VkCommandBuffer buffers[] = { buffer->vulkanHandle };

	vkFreeCommandBuffers(dev, pool->vulkanHandle, 1, buffers);
	buffer->state = CBState::not_allocated;
	buffer->vulkanHandle = VK_NULL_HANDLE;

	delete buffer;
}

void vtek::command_pool_free_buffers(
	vtek::CommandPool* pool, std::vector<vtek::CommandBuffer*>& buffers,
	vtek::Device* device)
{
	if (buffers.size() == 0) { return; }

	for (auto buf : buffers)
	{
		if (buf->state == CBState::pending)
		{
			vtek_log_error(
				"Command buffer(s) cannot be freed from pending state!");
			return;
		}
	}

	std::vector<VkCommandBuffer> handles;
	for (uint32_t i = 0; i < buffers.size(); i++)
	{
		handles.push_back(buffers[i]->vulkanHandle);

		if (handles[i] != nullptr) { delete buffers[i]; }
	}
	vkFreeCommandBuffers(
		vtek::device_get_handle(device), pool->vulkanHandle,
		handles.size(), handles.data());

	buffers.clear();
}

bool vtek::command_pool_reset_buffer(
	vtek::CommandPool* pool, vtek::CommandBuffer* buffer)
{
	// TODO: How do we measure this?
	// TODO: Perhaps require use of a semaphore - is it worth it?
	const auto state = buffer->state;
	if (state == CBState::pending)
	{
		vtek_log_error("Command buffer cannot be reset from pending state!");
		return false;
	}
	if (state == CBState::initial)
	{
		return true;
	}
	if (!pool->allowIndividualBufferReset)
	{
		vtek_log_error(
			"Command pool was not created with the individual reset flag -- {}",
			"cannot reset command buffer!");
		return false;
	}

	// NOTE: `flags` may be the VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT
	// flag, which specifies that memory resources owned by the command buffer
	// should be returned to the parent command pool.
	VkCommandBufferResetFlags flags = 0;
	VkResult result = vkResetCommandBuffer(buffer->vulkanHandle, flags);
	if (result != VK_SUCCESS)
	{
		vtek_log_error("Failed to reset command buffer!");
		return false;
	}

	buffer->state = CBState::initial;
	return false;
}
