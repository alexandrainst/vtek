#pragma once

#include <vulkan/vulkan.h>

#include "vtek_types.hpp"
#include "vtek_vulkan_handles.hpp"


namespace vtek
{
	struct BufferCreateInfo
	{
		VkDeviceSize bufferSize {0UL};
		// TODO: stride, view, usage flags, memory properties, mappable
	};

	struct Buffer; // opaque handle

	Buffer* buffer_create();
	void buffer_destroy(Buffer* buffer);

	// If a buffer is host visible it can be directly memory-mapped.
	bool buffer_is_host_visible(Buffer* buffer);

	// TODO: View into the buffer for binding purposes (SRV,UAV,VB,etc.)
	void buffer_get_view_for_binding_purposes();


	// ==================== //
	// === Proposed API === //
	// ==================== //
	struct BufferAllocator;

	enum class BufferAllocatorType
	{
		linear,
		circular,
		pool,
		heap // TODO: vma supports this?
	};

	struct BufferAllocatorInfo
	{
		BufferAllocatorType type {BufferAllocatorType::linear};
		uint64_t poolSize {0UL}; // Ignored when type is not pool.
	};

	BufferAllocator* buffer_allocator_create();
	void buffer_allocator_destroy();

	// Maybe the buffer maintains its own staging buffer, or maybe the buffer can be
	// host-mapped because that was specified at buffer creation.
	// In any case, we should not care about _how_ the contents get updated, just when
	// and what we update it with.
	// The "how" should be cared about, and specified, when the buffer is created, by
	// specifying flags and options for its usage.
	// And if the buffer does not support the desired update, then contents are not
	// updated, and an error is returned.
	bool buffer_write_data(Buffer* buffer, void* data, uint64_t size, Device* device);

	// Read contents from buffer info `dest` std::vector. Again, we don't care how
	// this gets done, just that it works and that we call the function now.
	bool buffer_read_data(Buffer* buffer, std::vector<char>& dest, Device* device);

	struct BufferRegion
	{
		uint64_t offset {0UL};
		uint64_t size {0UL};
	};

	// Copy the contents of one buffer into another buffer. No memory mapping is required,
	// instead a single-use transfer command buffer is queued.
	// TODO: Wait for fence?
	bool buffer_copy(Buffer* src, BufferRegion* srcRegion, Buffer* dest, uint64_t destOffset);


	enum class BufferUsage
	{
		// Content of the buffer is set once and then never changed.
		static_usage,
	};

	struct BufferInfo
	{
		EnumBitmask usageFlags {};
	};

	buffer_create_uniforms(
		size, int numBuffers, std /* uniform (std 140) ?*/ );


}
