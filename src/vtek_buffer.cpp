#include "vtek_vulkan.pch"
#include "vtek_buffer.hpp"

#include "impl/vtek_vma_helpers.hpp"
#include "vtek_command_buffer.hpp"
#include "vtek_command_scheduler.hpp"
#include "vtek_device.hpp"
#include "vtek_logging.hpp"


/* helper functions */
static bool do_map_and_copy(
	vtek::Buffer* buffer, void* data, const vtek::BufferRegion* region)
{
	void* mappedPtr = vtek::allocator_buffer_map(buffer); // TODO: Offset!
	if (mappedPtr == nullptr)
	{
		vtek_log_error("Failed to map the buffer -- cannot write data!");
		return false;
	}

	memcpy(mappedPtr, data, region->size);

	// Flush if the buffer is not HOST_COHERENT
	auto memProps = buffer->memoryProperties;
	if (!memProps.has_flag(vtek::MemoryProperty::host_coherent))
	{
		vtek::allocator_buffer_flush(buffer, region);
	}

	vtek::allocator_buffer_unmap(buffer);

	return true;
}

static bool do_schedule_transfer(
	vtek::Buffer* source, vtek::Buffer* destination,
	const vtek::BufferRegion* region, vtek::Device* device)
{
	auto scheduler = vtek::device_get_command_scheduler(device);
	auto commandBuffer =
		vtek::command_scheduler_begin_transfer(scheduler, device);
	if (commandBuffer == nullptr)
	{
		vtek_log_error(
			"Failed to begin single-use transfer command buffer -- {}",
			"cannot write data to buffer!");
		return false;
	}

	// TODO: Interface for command buffer operations inside vtek!
	//vtek::cmd_copy_buffer(...); // etc.
	VkCommandBuffer cmdBuf = vtek::command_buffer_get_handle(commandBuffer);
	VkBuffer srcBuf = source->vulkanHandle;
	VkBuffer dstBuf = destination->vulkanHandle;

	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = region->offset;
	copyRegion.size = region->size;
	vkCmdCopyBuffer(cmdBuf, srcBuf, dstBuf, 1, &copyRegion);

	return vtek::command_scheduler_submit_transfer(
		scheduler, commandBuffer, device);
}



/* interface */
vtek::Buffer* vtek::buffer_create(
	const vtek::BufferInfo* info, vtek::Device* device)
{
	auto buffer = new vtek::Buffer;
	vtek::Allocator* allocator = vtek::device_get_allocator(device);
	if (allocator == nullptr)
	{
		vtek_log_error("Device does not have a default allocator -- {}",
		               "cannot create buffer!");
		delete buffer;
		return nullptr;
	}

	if (!vtek::allocator_buffer_create(allocator, info, buffer))
	{
		vtek_log_error("Failed to create buffer!");
		delete buffer;
		return nullptr;
	}

	bool createStagingBuffer
		= !info->disallowInternalStagingBuffer
		& !info->requireHostVisibleStorage
		& (info->writePolicy != vtek::BufferWritePolicy::write_once);
	if (createStagingBuffer)
	{
		buffer->stagingBuffer = new vtek::Buffer;
		buffer->stagingBuffer->stagingBuffer = nullptr;

		vtek::BufferInfo stagingInfo{};
		stagingInfo.size = info->size;
		stagingInfo.requireHostVisibleStorage = true;
		stagingInfo.usageFlags = vtek::BufferUsageFlag::transfer_src;

		if (!vtek::allocator_buffer_create(allocator, &stagingInfo, buffer->stagingBuffer))
		{
			vtek_log_error("Failed to create staging buffer for buffer!");
			delete buffer->stagingBuffer;
			buffer->stagingBuffer = nullptr;
		}
	}

	buffer->allocator = allocator;
	return buffer;
}

void vtek::buffer_destroy(vtek::Buffer* buffer)
{
	if (buffer == nullptr) return;

	vtek::allocator_buffer_destroy(buffer);
	if (buffer->stagingBuffer != nullptr)
	{
		vtek::allocator_buffer_destroy(buffer->stagingBuffer);
	}

	buffer->allocator = nullptr;
	buffer->stagingBuffer = nullptr;

	delete buffer;
}

VkBuffer vtek::buffer_get_handle(const vtek::Buffer* buffer)
{
	return buffer->vulkanHandle;
}

bool vtek::buffer_write_data(
	vtek::Buffer* buffer, void* data, const vtek::BufferRegion* region,
	vtek::Device* device)
{
	VkDeviceSize writeOffset = region->offset;
	VkDeviceSize writeSize =
		(region->size == VK_WHOLE_SIZE) ? buffer->size : region->size;
	if (writeSize > buffer->size)
	{
		vtek_log_warn("vtek::buffer_write_data: {} -- {}",
		              "Data size is larger than capacity of buffer",
		              "output will be clamped!");
		writeSize = buffer->size;
	}
	if (writeOffset >= buffer->size)
	{
		vtek_log_error("vtek::buffer_write_data: {} -- {}"
		              "Offset is outside the boundaries of the buffer",
		              "no data will be written!");
		return false;
	}
	if (writeSize + region->offset > buffer->size)
	{
		vtek_log_warn("vtek::buffer_write_data: {} -- {}",
		              "Region size+offset exceeds the buffer boundaries",
		              "output will be clamped!");
		writeSize = buffer->size - writeOffset;
	}

	// Now for choices...
	auto memProps = buffer->memoryProperties;
	vtek::BufferRegion finalRegion{ writeOffset, writeSize }; // possibly corrected

	// 1) Buffer is HOST_VISIBLE - just map directly.
	if (memProps.has_flag(vtek::MemoryProperty::host_visible))
	{
		return do_map_and_copy(buffer, data, &finalRegion);
	}

	// 2) Buffer has a staging buffer - map to that, then transfer queue.
	else if (buffer->stagingBuffer != nullptr)
	{
		if (!do_map_and_copy(buffer->stagingBuffer, data, &finalRegion))
		{
			return false;
		}

		return do_schedule_transfer(
			buffer->stagingBuffer, buffer, &finalRegion, device);
	}

	// 3) Create a temporary staging buffer - map to that, then transfer queue.
	else
	{
		vtek::BufferInfo stagingInfo{};
		stagingInfo.size = finalRegion.size;
		stagingInfo.requireHostVisibleStorage = true;
		stagingInfo.disallowInternalStagingBuffer = true;
		stagingInfo.usageFlags = vtek::BufferUsageFlag::transfer_src;

		// TODO: Dynamic allocation of `tempStaging` is not really necessary here.
		vtek::Buffer* tempStaging = new vtek::Buffer;
		tempStaging->stagingBuffer = nullptr;

		vtek::Allocator* allocator = vtek::device_get_allocator(device);
		if (!vtek::allocator_buffer_create(allocator, &stagingInfo, tempStaging))
		{
			vtek_log_error("Failed to create temporary staging buffer -- {}",
			               "cannot write data to buffer!");
			delete tempStaging;
			return false;
		}

		if (!do_map_and_copy(tempStaging, data, &finalRegion))
		{
			vtek::allocator_buffer_destroy(tempStaging);
			delete tempStaging;
			return false;
		}

		if (!do_schedule_transfer(tempStaging, buffer, &finalRegion, device))
		{
			vtek::allocator_buffer_destroy(tempStaging);
			delete tempStaging;
			return false;
		}

		vtek::allocator_buffer_destroy(tempStaging);
		delete tempStaging;

		return true;
	}
}



// ===================== //
// === Cache control === //
// ===================== //

// TODO: Use these notes for something or delete?
// Any memory type that doesn't have HOST_COHERENT flag needs manual cache constrol:
// - vkInvalidateMappedMemoryRanges before read on CPU.
// - vkFlushMappedMemoryRanges after write on CPU.
