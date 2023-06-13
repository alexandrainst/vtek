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



// ===================== //
// === Cache control === //
// ===================== //

// Any memory type that doesn't have HOST_COHERENT flag needs manual cache constrol:
// - vkInvalidateMappedMemoryRanges before read on CPU.
// - vkFlushMappedMemoryRanges after write on CPU.
