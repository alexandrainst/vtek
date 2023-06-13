#pragma once

#include "vtek_buffer.hpp"
#include "vtek_vulkan_handles.hpp"


namespace vtek
{
	struct AllocatorInfo
	{
		int dummyMember;
	};

	struct Allocator; // opaque handle


	Allocator* allocator_create(
		Device* device, Instance* instance, const AllocatorInfo* info);
	void allocator_destroy(Allocator* allocator);

	// This function is called when the device is created so that each
	// device manages its own allocator.
	Allocator* allocator_create_default(Device* device, Instance* instance);


	// ========================= //
	// === Buffer management === //
	// ========================= //

	allocator_create_buffer();



}
