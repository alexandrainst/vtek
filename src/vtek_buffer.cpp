#include "vtek_buffer.hpp"

#include "impl/vtek_vma_helpers.hpp"
#include "vtek_device.hpp"
#include "vtek_logging.hpp"


/* interface */
vtek::Buffer* vtek::buffer_create(
	const vtek::BufferInfo* info, vtek::Device* device)
{
	vtek::Allocator* allocator = vtek::device_get_allocator(device);
	auto [bufHandle, vmaHandle] = vtek::allocator_buffer_create(allocator, info);
	if (bufHandle == VK_NULL_HANDLE || vmaHandle == VK_NULL_HANDLE)
	{
		vtek_log_error("Failed to create buffer!");
		return nullptr;
	}

	auto buffer = new vtek::Buffer;
	buffer->vulkanHandle = bufHandle;
	buffer->vmaHandle = vmaHandle;

	bool createStagingBuffer
		= !info->disallowInternalStagingBuffer
		& !info->requireHostVisibleStorage
		& (info->writePolicy != vtek::BufferWritePolicy::write_once);
	if (createStagingBuffer)
	{
		vtek::BufferInfo stagingInfo{};
		stagingInfo.size = info->size;
		stagingInfo.requireHostVisibleStorage = true;
		stagingInfo.usageFlags = vtek::BufferUsageFlag::transfer_src;
		auto [stBufHdl, stVmaHdl] =
			vtek::allocator_buffer_create(allocator, info);

		buffer->stagingBuffer = new vtek::Buffer;
		buffer->stagingBuffer->vulkanHandle = stBufHdl;
		buffer->stagingBuffer->vmaHandle = stVmaHdl;
		buffer->stagingBuffer->allocator = allocator;
		buffer->stagingBuffer->hostMappingEnabled = true;
		// TODO: How do we know if it is HOST_COHERENT ?
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
