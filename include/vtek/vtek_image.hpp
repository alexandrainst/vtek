#pragma once

#include <vulkan/vulkan.h>
#include <cstdint>
#include <string_view>

#include "vtek_format_support.hpp"
#include "vtek_object_handles.hpp"
#include "vtek_types.hpp"
#include "vtek_vulkan_types.hpp"


namespace vtek
{
	// =========================== //
	// === Image creation info === //
	// =========================== //

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

	enum class ImageAspectFlag : uint32_t
	{
		// Possible bit-masking of the 3 major attachment types.
		color    = 0x0001,
		depth    = 0x0002,
		stencil  = 0x0004,
		// Specifies the metadata aspect used for sparse resource operations.
		metadata = 0x0008,
		// Image aspects for multi-planar image formats.
		// NOTE: plane 0/1 requires Vulkan >= 1.1, plane 2 requires Vulkan >= 1.3.
		// NOTE: Multi-planar formats are probably only useful for video decoding.
		plane_0  = 0x0010,
		plane_1  = 0x0020,
		plane_2  = 0x0040
	};

	enum class ImageInitialLayout
	{
		// Default initial layout. It means that the image is neither valid
		// to write to OR read from, and that a layout transition MUST be
		// performed before using the image for any purposes.
		undefined,
		// Pre-initialized image layout is used as initial layout if an image
		// should be written to by the host, and signifies that the image data
		// can be written to memory immediately without first executing a
		// layout transition.
		// NOTE: Pre-initialized is ONLY valid with linear tiling, and a manual
		// layout transition into optimal tiling MUST be performed before the
		// image may be read from. This severely limits the amount of supported
		// image formats, since optimal tiling has way better support.
		// NOTE: This implies the image to be located in HOST_VISIBLE memory,
		// as only host mappings may modify the contents initially.
		// NOTE: Hence, for most purposes this initial layout is not recommended.
		preinitialized
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

	enum class ImageSharingMode
	{
		exclusive, concurrent
	};


	// ========================= //
	// === Utility functions === //
	// ========================= //

	VkImageLayout get_image_layout(ImageLayout layout);


	// ========================= //
	// === Image2D interface === //
	// ========================= //

	struct Image2DViewInfo
	{
		uint32_t baseMipLevel {0}; // aka. mip-lod-bias
		uint32_t baseArrayLayer {0}; // only applicable for image arrays.

		// Optional aspect flags, specifies which aspect(s) of the image are
		// included in a view created on the image.
		// NOTE: The aspect flags MUST be compatible with the format specified
		// for image creation, but MAY be a valid subset e.g. specifying only
		// depth for a depth/stencil image used as an attachment.
		EnumBitmask<ImageAspectFlag> aspectFlags {0U};
	};

	struct Image2DInfo
	{
		// Some resources, e.g. render targets, depth-stencil, UAV, very large
		// buffers and images, or large allocations that need dynamic resizing,
		// can be given a dedicated allocation, instead of being suballocated
		// from a bigger block.
		bool requireDedicatedAllocation {false};

		VkExtent2D extent {0U, 0U};

		// Format specification:
		// 1) Directly specify an image format (not recommended,
		//    GPU support not guaranteed!)
		// 2) Query a known supported format (recommended path)
		Format format {Format::undefined};
		SupportedFormat supportedFormat {};

		// Specify how the image should be used. At least one flag must be set.
		EnumBitmask<ImageUsageFlag> usageFlags {0U};
		ImageInitialLayout initialLayout {ImageInitialLayout::undefined};
		// TODO: If mipmaps then specify number of mip levels (or compute automatically)?
		bool useMipmaps {false};
		// The `samples` flag is related to multisampling. This is only relevant
		// for images that will be used as attachments,
		MultisampleType multisampling {MultisampleType::none};

		// This is only relevant for images that are shared between multiple
		// queue families, in which case sharing mode should be `concurrent`.
		// NOTE: When sharing mode is `exclusive`, the sharing queues are ignored.
		ImageSharingMode sharingMode {ImageSharingMode::exclusive};
		std::vector<const Queue*> sharingQueues;

		// If an image view should be created automatically (avoid creating it
		// later manually). This should be preferred for most cases, why its
		// default is set to `true`.
		bool createImageView {true};
		Image2DViewInfo imageViewInfo {};
	};

	struct Image2DLoadInfo
	{
		bool loadSRGB {true};

		// TODO: Description and rationale.
		bool forceAlphaPremultiply {false};

		// TODO: Implementation (both blit and compute)
		bool createMipmaps {false};
		uint32_t maxNumMipmaps {UINT32_MAX};
		uint32_t baseMipLevel {0}; // aka. mip-lod-bias
	};


	// ======================= //
	// === Image interface === //
	// ======================= //

	Image2D* image2d_create(const Image2DInfo* info, Device* device);
	void image2d_destroy(Image2D* image, Device* device);

	// TODO: Alternative image creation, using a specific allocator
	Image2D* image2d_create(const Image2DInfo* info, Allocator* allocator);

	Image2D* image2d_load(
		const Image2DLoadInfo* info, const Directory* directory,
		std::string_view filename, Device* device);


	// ======================== //
	// === Image properties === //
	// ======================== //

	VkImage image2d_get_handle(const Image2D* image);
	VkImageView image2d_get_view_handle(const Image2D* image);
	VkFormat image2d_get_format(const Image2D* image);
}
