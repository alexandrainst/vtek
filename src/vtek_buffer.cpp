#include "vtek_buffer.hpp"

#include "impl/vtek_vma_helpers.hpp"
#include "vtek_device.hpp"
#include "vtek_logging.hpp"


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
	vtek_log_info("Created buffer!"); // TODO: Example output ?

	bool createStagingBuffer
		= !info->disallowInternalStagingBuffer
		& !info->requireHostVisibleStorage
		& (info->writePolicy != vtek::BufferWritePolicy::write_once);
	if (createStagingBuffer)
	{
		buffer->stagingBuffer = new vtek::Buffer;

		vtek::BufferInfo stagingInfo{};
		stagingInfo.size = info->size;
		stagingInfo.requireHostVisibleStorage = true;
		stagingInfo.usageFlags = vtek::BufferUsageFlag::transfer_src;

		if (!vtek::allocator_buffer_create(allocator, info, buffer->stagingBuffer))
		{
			vtek_log_error("Failed to create staging buffer for buffer!");
			delete buffer->stagingBuffer;
			buffer->stagingBuffer = nullptr;
		}
		vtek_log_info("Created staging buffer!"); // TODO: Example output ?
	}

	buffer->allocator = allocator;
	return buffer;
}

void vtek::buffer_destroy(vtek::Buffer* buffer)
{
	if (buffer == nullptr) return;

	vtek::allocator_buffer_destroy(buffer->allocator, buffer);
	buffer->allocator = nullptr;
	buffer->stagingBuffer = nullptr;

	delete buffer;
}

bool vtek::buffer_write_data(
	vtek::Buffer* buffer, void* data, uint64_t size, vtek::Device* device)
{
	VkDeviceSize writeSize = size;
	if (writeSize > buffer->size)
	{
		vtek_log_warn("vtek::buffer_write_data: {} -- {}",
		              "Data size is larger than capacity of buffer",
		              "output will be clamped!");
		writeSize = buffer->size;
	}

	// Now for choices...

	// 1) Buffer is HOST_VISIBLE - just map directly.
	if (buffer->memoryProperties.has_flag(vtek::MemoryProperty::host_visible))
	{
		vtek_log_trace("Buffer is HOST_VISIBLE - map to it directly!");

		return false;
	}

	// 2) Buffer has a staging buffer - map to that, then transfer queue.
	if (buffer->stagingBuffer != nullptr)
	{
		vtek_log_trace("Buffer has a staging buffer - map to that, then transfer!");

		return false;
	}

	// 3) Create a temporary staging buffer - map to that, then transfer queue.
	vtek_log_trace("Buffer doesn't have staging buffer - create a temporary one!");

	return false;
}



// ===================== //
// === Cache control === //
// ===================== //

// Any memory type that doesn't have HOST_COHERENT flag needs manual cache constrol:
// - vkInvalidateMappedMemoryRanges before read on CPU.
// - vkFlushMappedMemoryRanges after write on CPU.
