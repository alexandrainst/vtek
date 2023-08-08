#pragma once

#include <vulkan/vulkan.h>
#include <cstdint>

#include "vtek_object_handles.hpp"
#include "vtek_types.hpp"


namespace vtek
{
	enum class ImageChannels : uint32_t
	{
		channels_1 = 1,
		channels_2 = 2,
		channels_3 = 3,
		channels_4 = 4
	};

	enum class ImagePixelStorageFormat
	{
		unorm,
		snorm,
		uint8,
		sint8,
		float16,
		float32,
		pack_unorm16,
		pack_float16,
		pack_float32
	};

	enum class ImageUsageFlag : uint32_t
	{
		transfer_src             = 0x0001,
		transfer_dst             = 0x0002,
		sampled                  = 0x0004,
		storage                  = 0x0008,
		color_attachment         = 0x0010,
		depth_stencil_attachment = 0x0020,
		transient_attachment     = 0x0040,
		input_attachment         = 0x0080
	};

	enum class ImageLayout
	{
		// Standard values, provided by Vulkan >= 1.0
		undefined,
		general,
		color_attachment_optimal,
		depth_stencil_attachment_optimal,
		depth_stencil_readonly_optimal,
		shader_readonly_optimal,
		transfer_src_optimal,
		transfer_dst_optimal,
		preinitialized,

		// Provided by Vulkan >= 1.1
		depth_readonly_stencil_attachment_optimal,
		depth_attachment_stencil_readonly_optimal,

		// Provided by Vulkan >= 1.2
		depth_attachment_optimal,
		depth_readonly_optimal,
		stencil_attachment_optimal,
		stencil_readonly_optimal,

		// Provided by Vulkan >= 1.3
		readonly_optimal,
		attachment_optimal
	};

	struct Image2DInfo
	{
		// Some resources, e.g. render targets, depth-stencil, UAV, very large
		// buffers and images, or large allocations that need dynamic resizing,
		// can be given a dedicated allocation, instead of being suballocated
		// from a bigger block.
		bool requireDedicatedAllocation {false};

		VkExtent2D size {0U, 0U};

		// NOTE: Format specification: either directly choose a `VkFormat` type,
		// or let vtek figure out a supported format which most closely matches
		// the provided parameters.
		VkFormat format {VK_FORMAT_UNDEFINED};
		ImageChannels channels {ImageChannels::channels_4};
		ImagePixelStorageFormat pixelStorageFormat {ImagePixelStorageFormat::unorm};
		bool imageStorageSRGB {false};
		bool swizzleBGR {false}; // TODO: When, why, and how?

		// Specify how the image should be used. At least one flag must be set.
		EnumBitmask<ImageUsageFlag> usageFlags {0U};

		ImageLayout initialLayout {ImageLayout::undefined};

	};

	Image2D* image2d_create(const Image2DInfo* info, Device* device);
	void image2d_destroy(Image2D* image);

	// TODO: Alternative image creation, using a specific allocator
	Image2D* image2d_create(const Image2DInfo* info, Allocator* allocator);

	VkImage image2d_get_handle(const Image2D* image);
	VkImageView image2d_get_view_handle(const Image2D* image);


	// ======================== //
	// === Image operations === //
	// ======================== //

	bool image_transition_layout(
		Image2D* image, ImageLayout oldLayout, ImageLayout newLayout,
		Device* device);
}
