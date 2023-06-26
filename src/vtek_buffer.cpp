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
		vtek_log_trace("create staging buffer...");
		buffer->stagingBuffer = new vtek::Buffer;
		buffer->stagingBuffer->stagingBuffer = nullptr;

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

	vtek::allocator_buffer_destroy(buffer);
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
		void* mappedPtr = vtek::allocator_buffer_map(buffer); // TODO: Offset!
		if (mappedPtr == nullptr)
		{
			vtek_log_error("Failed to map the buffer -- cannot write data!");
			return false;
		}

		memcpy(mappedPtr, data, finalRegion.size);

		// Flush if the buffer is not HOST_COHERENT
		if (!memProps.has_flag(vtek::MemoryProperty::host_coherent))
		{
			vtek::allocator_buffer_flush(buffer, &finalRegion);
		}

		vtek::allocator_buffer_unmap(buffer);

		return true;
	}

	// 2) Buffer has a staging buffer - map to that, then transfer queue.
	if (buffer->stagingBuffer != nullptr)
	{
		vtek_log_trace("Buffer has a staging buffer - map to that, then transfer!");

		auto scheduler = vtek::device_get_command_scheduler(device);

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
