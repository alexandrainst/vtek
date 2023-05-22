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
