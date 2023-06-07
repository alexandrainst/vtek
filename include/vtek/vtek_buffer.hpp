#pragma once

#include <vulkan/vulkan.h>
#include <cstddef>
#include <vector>

#include "vtek_types.hpp"
#include "vtek_vulkan_handles.hpp"


namespace vtek
{
	enum class BufferWritePolicy
	{
		// Content of the buffer is set once and then never changed.
		write_once,
		// Contents will be changed on occasion, which will hint at internally
		// managed staging memory.
		overwrite_sometimes,
		// Contents will be changed often or every frame, which will probably
		// result in a buffer created without DEVICE_LOCAL bit, to minimize
		// copying.
		// NOTE: This is not recommended for larger buffers.
		overwrite_often
	};

	enum class BufferUsageFlag
	{
		transfer_src          = 0x0001u,
		transfer_dst          = 0x0002u,
		uniform_texel_buffer  = 0x0004u,
		storage_texel_buffer  = 0x0008u,
		uniform_buffer        = 0x0010u,
		storage_buffer        = 0x0020u,
		index_buffer          = 0x0040u,
		vertex_buffer         = 0x0080u,
		indirect_buffer       = 0x0100u,
		shader_device_address = 0x0200u,
	};

	struct BufferInfo
	{
		VkDeviceSize size {0UL};
		// TODO: stride, view, usage flags, memory properties, mappable

		// If the buffer should reside on system memory, or on GPU memory
		// accessible by the CPU. This supports memory mapping between
		// CPU/GPU, but is slower for the GPU to read.
		bool requireHostVisibleStorage {false};

		// If set, the buffer will not internally create a staging buffer, even
		// when its contents is overwritten often. This may be used to prevent
		// additional staging memory usage, or if several buffers share the same
		// staging memory - which must be synchronized externally!
		bool disallowInternalStagingBuffer {false};

		// Some resources, e.g. render targets, depth-stencil, UAV, very large
		// buffers and images, or large allocations that need dynamic resizing,
		// can be given a dedicated allocation, instead of being suballocated
		// from a bigger block.
		bool requireDedicatedAllocation {false};

		// How often the contents of the buffer is overwritten. If this happens
		// often, the buffer will maintain an additional staging buffer
		// internally.
		BufferWritePolicy writePolicy {BufferWritePolicy::overwrite_once};

		// Specify how the buffer should be used. At least one flag must be set.
		EnumBitmask<BufferUsageFlag> usageFlags {};
	}
;
	struct Buffer; // opaque handle


	Buffer* buffer_create(const BufferInfo* info);
	void buffer_destroy(Buffer* buffer);

	// If a buffer is host visible it can be directly memory-mapped.
	bool buffer_is_host_visible(Buffer* buffer);

	// TODO: View into the buffer for binding purposes (SRV,UAV,VB,etc.)
	void buffer_get_view_for_binding_purposes();


	// ==================== //
	// === Proposed API === //
	// ==================== //

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
	bool buffer_read_data(Buffer* buffer, std::vector<std::byte>& dest, Device* device);

	struct BufferRegion
	{
		uint64_t offset {0UL};
		uint64_t size {0UL};
	};

	// Copy the contents of one buffer into another buffer. No memory mapping is required,
	// instead a single-use transfer command buffer is queued.
	// TODO: Wait for fence?
	bool buffer_copy(Buffer* src, BufferRegion* srcRegion, Buffer* dest, uint64_t destOffset);


	// buffer_create_uniforms(
	// 	size, int numBuffers, std /* uniform (std 140) ?*/ );
}
