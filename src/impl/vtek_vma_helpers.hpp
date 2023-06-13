#pragma once

// VMA include
#include "vtek_vulkan.pch"
#include "vtek_vulkan_handles.hpp"


namespace vtek
{
	struct Buffer
	{
		VkBuffer vulkanHandle {VK_NULL_HANDLE};
		VmaAllocation vmaHandle {VK_NULL_HANDLE};

		EnumBitmask<MemoryProperty> memoryProperties {};

		// If host mapping should be enabled and the buffer update policy is set
		// to "frequently", then the buffer should manage its own staging memory.
		vtek::Buffer* stagingBuffer {nullptr};

		// The buffer knows who created it.
		// Will be used for all subsequent operations on the buffer, including
		// its deletion.
		vtek::Allocator* allocator {nullptr};
	};

	// ========================= //
	// === Buffer management === //
	// ========================= //

	std::pair<VkBuffer, VmaAllocation> allocator_buffer_create(
		Allocator* allocator, const BufferInfo* info);
	void allocator_buffer_destroy(Allocator* allocator);
}
