#pragma once

#include <vulkan/vulkan.h>


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
}
