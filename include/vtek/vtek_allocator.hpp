#pragma once

#include <utility>
#include <vulkan/vulkan.h>

#include "vtek_buffer.hpp"
#include "vtek_vulkan_handles.hpp"


namespace vtek
{
	enum class AllocatorType
	{
		linear,
		circular,
		pool,
		heap // TODO: vma supports this?
	};

	struct AllocatorInfo
	{
		AllocatorType type {AllocatorType::linear};
		uint64_t poolSize {0UL}; // Ignored when type is not pool.
	};


	Allocator* allocator_create(
		Device* device, const Instance* instance, const AllocatorInfo* info);
	void allocator_destroy(Allocator* allocator);

	// This function is called when the device is created so that each
	// device manages its own allocator.
	Allocator* allocator_create_default(Device* device, const Instance* instance);
}
