#pragma once

// VMA include
#include "vtek_vulkan.pch"
#include "vtek_vulkan_handles.hpp"


namespace vtek
{
	/* internal helper types */
	// TODO: Do we want this?
	enum class MemoryProperty : uint32_t
	{
		device_local     = 0x0001U,
		host_visible     = 0x0002U,
		host_coherent    = 0x0004U,
		host_cached      = 0x0008U,
		lazily_allocated = 0x0010U,
		memory_protected = 0x0020U
	};

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

		bool hostMappingEnabled {false};
		bool hostCoherent {false};
	};

	// ========================= //
	// === Buffer management === //
	// ========================= //

	std::pair<VkBuffer, VmaAllocation> allocator_buffer_create(
		Allocator* allocator, const BufferInfo* info);
	void allocator_buffer_destroy(Allocator* allocator, Buffer* buffer);
}
