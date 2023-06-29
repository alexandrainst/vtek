#include "vtek_vulkan.pch"
#include "vtek_command_buffer.hpp"

#include "impl/vtek_command_buffer_struct.hpp"
#include "vtek_command_pool.hpp"
#include "vtek_device.hpp"
#include "vtek_logging.hpp"
// TODO: No longer use sAllocator ?
//#include "impl/vtek_host_allocator.hpp"

using CBState = vtek::CommandBufferStateType;


/* host allocator */
// TODO: Because of the high memory requirement, perhaps this particular allocator
//       should store a pointer to a vtek::Device instead, as an optimization.
// OKAY: This should be done!
// TODO: No longer use sAllocator ?
//static vtek::HostAllocator<vtek::CommandBuffer> sAllocator("command_buffer");



/* interface */
vtek::CommandBuffer* vtek::command_buffer_create(
	const vtek::CommandBufferCreateInfo* info, vtek::CommandPool* pool, vtek::Device* device)
{
	auto commandBuffer = new vtek::CommandBuffer();

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.pNext = nullptr;
	allocInfo.commandPool = vtek::command_pool_get_handle(pool);
	allocInfo.level = (info->isSecondary)
		? VK_COMMAND_BUFFER_LEVEL_SECONDARY
		: VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	VkResult allocResult = vkAllocateCommandBuffers(
		vtek::device_get_handle(device), &allocInfo, &commandBuffer->vulkanHandle);
	if (allocResult != VK_SUCCESS)
	{
		vtek_log_error("Failed to allocate command buffer!");
		delete commandBuffer;
		return nullptr;
	}

	commandBuffer->isSecondary = info->isSecondary;
	commandBuffer->state = CBState::initial;
	return commandBuffer;
}

std::vector<vtek::CommandBuffer*> vtek::command_buffer_create(
	const vtek::CommandBufferCreateInfo* info, uint32_t createCount,
	vtek::CommandPool* pool, vtek::Device* device)
{
	// TODO: Try something different here with memory allocation.
	auto allocations = new vtek::CommandBuffer[createCount];
	std::vector<vtek::CommandBuffer*> commandBuffers(createCount, VK_NULL_HANDLE);
	for (uint32_t i = 0; i < createCount; i++)
	{
		commandBuffers[i] = &(allocations[i]);
	}

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.pNext = nullptr;
	allocInfo.commandPool = vtek::command_pool_get_handle(pool);
	allocInfo.level = (info->isSecondary)
		? VK_COMMAND_BUFFER_LEVEL_SECONDARY
		: VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = createCount;

	std::vector<VkCommandBuffer> vulkanHandles(createCount, VK_NULL_HANDLE);

	VkResult allocResult = vkAllocateCommandBuffers(
		vtek::device_get_handle(device), &allocInfo, vulkanHandles.data());
	if (allocResult != VK_SUCCESS)
	{
		vtek_log_error("Failed to allocate command buffer!");
		delete[] allocations;
		return {};
	}

	bool reset = vtek::command_pool_allow_individual_reset(pool);
	bool secondary = info->isSecondary;

	for (uint32_t i = 0; i < createCount; i++)
	{
		commandBuffers[i]->vulkanHandle = vulkanHandles[i];
		commandBuffers[i]->poolHandle = vtek::command_pool_get_handle(pool); // TODO: ??
		commandBuffers[i]->supportsReset = reset;
		commandBuffers[i]->isSecondary = secondary;
		commandBuffers[i]->state = CBState::initial;
	}

	return commandBuffers;
}

void vtek::command_buffer_destroy(vtek::CommandBuffer* commandBuffer, vtek::Device* device)
{
	if (commandBuffer->state == CBState::pending) // TODO: How do we measure this?
	{
		vtek_log_error("Command buffer cannot be freed from pending state!");
		return;
	}

	VkDevice dev = vtek::device_get_handle(device);
	VkCommandPool pool = commandBuffer->poolHandle;
	const VkCommandBuffer buffers[] = { commandBuffer->vulkanHandle };

	vkFreeCommandBuffers(dev, pool, 1, buffers);
}

void vtek::command_buffer_destroy(
	std::vector<CommandBuffer*>& commandBuffers, vtek::Device* device)
{
	if (commandBuffers.size() == 0) { return; }

	for (auto buf : commandBuffers)
	{
		if (buf->state == CBState::pending)
		{
			vtek_log_error("Command buffer(s) cannot be freed from pending state!");
			return;
		}
	}

	std::vector<VkCommandBuffer> handles;
	for (auto buf : commandBuffers)
	{
		handles.push_back(buf->vulkanHandle);
	}
	vkFreeCommandBuffers(
		vtek::device_get_handle(device), commandBuffers[0]->poolHandle,
		handles.size(), handles.data());

	for (auto buf : commandBuffers)
	{
		buf->vulkanHandle = VK_NULL_HANDLE;
		buf->poolHandle = VK_NULL_HANDLE;
		buf->state = CBState::not_allocated;
	}

	delete[] commandBuffers.data();
	commandBuffers.clear();
}

VkCommandBuffer vtek::command_buffer_get_handle(vtek::CommandBuffer* commandBuffer)
{
	return commandBuffer->vulkanHandle;
}

VkCommandPool vtek::command_buffer_get_pool_handle(vtek::CommandBuffer* commandBuffer)
{
	return commandBuffer->poolHandle;
}

bool vtek::command_buffer_begin(vtek::CommandBuffer* commandBuffer)
{
	if (commandBuffer->state == CBState::pending) // TODO: How do we measure this?
	{
		vtek_log_error("Command buffer is in pending state and cannot begin recording!");
		return false;
	}
	if (commandBuffer->state == CBState::invalid) // TODO: How do we measure this?
	{
		vtek_log_error(
			"Command buffer is in invalid state and must be reset before recording can begin!");
		return false;
	}
	if (commandBuffer->state == CBState::recording) // TODO: How do we measure this?
	{
		vtek_log_warn(
			"Command buffer is already in recording state, so recorded commands may be lost!");
	}

	VkCommandBufferBeginInfo info{};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	info.pNext = nullptr;

	// VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT:
	//  - Specifies that each recording will only be submitted once, and then
	//    the command buffer will be reset and recorded again.
	// VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT:
	//  - Specifies that the secondary command buffer is considered to be entirely
	//    within a render pass. Ignored for primary command buffers.
	//  - `pInheritanceInfo.framebuffer` must be either VK_NULL_HANDLE or a valid
	//    VkFramebuffer that is compatible with `pInheritanceInfo.renderPass`.
	//  - pInheritanceInfo.renderPass must be either VK_NULL_HANDLE or a valid render pass.
	//  etc.
	// VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT:
	//  - Specifies that the command buffer can be resubmitted to a queue while in
	//    pending state, and recorded into multiple primary command buffers.
	// TODO: Add support for these flags?
	info.flags = 0;
	info.pInheritanceInfo = nullptr;

	VkCommandBufferInheritanceInfo inheritanceInfo{};
	if (commandBuffer->isSecondary)
	{
		vtek_log_fatal(
			"Secondary command buffer inheritance info is not implemented! Cannot begin recording.");
		return false;

		inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
		inheritanceInfo.pNext = nullptr;
		inheritanceInfo.renderPass = {}; // ??
		inheritanceInfo.subpass = 0; // ??
		inheritanceInfo.framebuffer = {}; // ??
		inheritanceInfo.occlusionQueryEnable = {}; // ??
		inheritanceInfo.queryFlags = 0; // ??
		inheritanceInfo.pipelineStatistics = {}; // ??

		inheritanceInfo = inheritanceInfo; // TODO: Just to eliminate compiler warning!
	}

	VkResult result = vkBeginCommandBuffer(commandBuffer->vulkanHandle, &info);
	if (result != VK_SUCCESS)
	{
		vtek_log_error("Failed to begin command buffer recording!");
		return false;
	}

	commandBuffer->state = CBState::recording;
	return true;
}

bool vtek::command_buffer_end(vtek::CommandBuffer* commandBuffer)
{
	if (commandBuffer->state != CBState::recording)
	{
		vtek_log_error(
			"Command buffer is not in recording state, so recording cannot end!");
		return false;
	}

	VkResult result = vkEndCommandBuffer(commandBuffer->vulkanHandle);
	if (result != VK_SUCCESS)
	{
		vtek_log_error("Failed to end command buffer recording!");
		return false;
	}

	commandBuffer->state = CBState::executable;
	return true;
}

bool vtek::command_buffer_reset(vtek::CommandBuffer* commandBuffer)
{
	// TODO: How do we measure this?
	// TODO: Perhaps require use of a semaphore - is it worth it?
	if (commandBuffer->state == CBState::pending)
	{
		vtek_log_error("Command buffer cannot be reset from pending state!");
		return false;
	}
	if (commandBuffer->state == CBState::initial)
	{
		return true;
	}
	if (!commandBuffer->supportsReset)
	{
		vtek_log_error("Command buffer cannot be reset from a pool not supporting buffer reset!");
		return false;
	}

	// NOTE: `flags` may be the VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT flag, which
	// specifies that memory resources owned by the command buffer should be returned to the
	// parent command pool.
	VkCommandBufferResetFlags flags = 0;
	VkResult result = vkResetCommandBuffer(commandBuffer->vulkanHandle, flags);
	if (result != VK_SUCCESS)
	{
		vtek_log_error("Failed to reset command buffer!");
		return false;
	}

	return true;
}
