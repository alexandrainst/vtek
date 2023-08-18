#pragma once

#include <cstdint>
#include <vulkan/vulkan.h>

#include "vtek_image.hpp"
#include "vtek_logging.hpp"


namespace vtek
{
	// TODO: Only for inspiration -- remove before compile!
	VkFormatFeatureFlags features
	= VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT
		| VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;

	enum class FormatFeatureFlag
	{
		SAMPLED_IMAGE = 0x00000001,
		STORAGE_IMAGE = 0x00000002,
		STORAGE_IMAGE_ATOMIC = 0x00000004,
		UNIFORM_TEXEL_BUFFER = 0x00000008,
		STORAGE_TEXEL_BUFFER = 0x00000010,
		STORAGE_TEXEL_BUFFER_ATOMIC = 0x00000020,
		VERTEX_BUFFER = 0x00000040,
		COLOR_ATTACHMENT = 0x00000080,
		COLOR_ATTACHMENT_BLEND = 0x00000100,
		DEPTH_STENCIL_ATTACHMENT = 0x00000200,
		BLIT_SRC = 0x00000400,
		BLIT_DST = 0x00000800,
		SAMPLED_IMAGE_FILTER_LINEAR = 0x00001000,

		// Provided by VK_VERSION_1_1
		TRANSFER_SRC = 0x00004000,
		TRANSFER_DST = 0x00008000,
		MIDPOINT_CHROMA_SAMPLES = 0x00020000,
		SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER = 0x00040000,
		SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER = 0x00080000,
		SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT = 0x00100000,
		SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_FORCEABLE = 0x00200000,
		DISJOINT = 0x00400000,
		COSITED_CHROMA_SAMPLES = 0x00800000,

		// Provided by VK_VERSION_1_2
		SAMPLED_IMAGE_FILTER_MINMAX = 0x00010000,
	};


	// Find first image format in the list that is supported by the
	// physical device. Returns a boolean indicating success.
	inline bool find_supported_image_format(
		VkPhysicalDevice physicalDevice, std::vector<VkFormat> prioritizedCandidates,
		VkImageTiling tiling, VkFormatFeatureFlags features, VkFormat* outFormat)
	{
		for (const VkFormat& format : prioritizedCandidates)
		{
			VkFormatProperties properties;
			vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &properties);

			if (tiling == VK_IMAGE_TILING_LINEAR &&
			    (properties.linearTilingFeatures & features) == features)
			{
				*outFormat = format;
				return true;
			}
			else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
			         (properties.optimalTilingFeatures & features) == features)
			{
				*outFormat = format;
				return true;
			}
		}

		return false;
	}

	// Find a suitable format matching provided parameters.


	// All these functions return `VK_FORMAT_UNDEFINED` in case a match
	// couldn't be found.

	// TODO: Better return bool instead?
	VkFormat get_format_color(
		const ImageFormatInfo* info, VkPhysicalDevice physDev,
		VkFormatFeatureFlags featureFlags);

	VkFormat get_format_color_srgb(
		const ImageFormatInfo* info, VkPhysicalDevice physDev,
		VkFormatFeatureFlags featureFlags);
}



/* internal helper functions */
static void vtek::get_format_color_channel_1(
	const vtek::ImageFormatInfo* info, std::vector<VkFormat>& priorities)
{
	if (info->channelSize == vtek::ImageChannelSize::channel_8)
	{
		switch (info->storageFormat)
		{
		case vtek::ImagePixelStorageFormat::unorm:
			priorities.push_back(VK_FORMAT_R8_UNORM);
			break;
		case vtek::ImagePixelStorageFormat::snorm:
			priorities.push_back(VK_FORMAT_R8_SNORM);
			break;
		case vtek::ImagePixelStorageFormat::uscaled:
			priorities.push_back(VK_FORMAT_R8_USCALED);
			break;
		case vtek::ImagePixelStorageFormat::sscaled:
			priorities.push_back(VK_FORMAT_R8_SSCALED);
			break;
		case vtek::ImagePixelStorageFormat::uint:
			priorities.push_back(VK_FORMAT_R8_UINT);
			break;
		case vtek::ImagePixelStorageFormat::sint:
			priorities.push_back(VK_FORMAT_R8_SINT);
			break;
		default:
			break;
		}
	}
	else if (info->channelSize == vtek::ImageChannelSize::channel_16)
	{
		switch (info->storageFormat)
		{
		case vtek::ImagePixelStorageFormat::unorm:
			priorities.push_back(VK_FORMAT_R16_UNORM);
			break;
		case vtek::ImagePixelStorageFormat::snorm:
			priorities.push_back(VK_FORMAT_R16_SNORM);
			break;
		case vtek::ImagePixelStorageFormat::uscaled:
			priorities.push_back(VK_FORMAT_R16_USCALED);
			break;
		case vtek::ImagePixelStorageFormat::sscaled:
			priorities.push_back(VK_FORMAT_R16_SSCALED);
			break;
		case vtek::ImagePixelStorageFormat::uint:
			priorities.push_back(VK_FORMAT_R16_UINT);
			break;
		case vtek::ImagePixelStorageFormat::sint:
			priorities.push_back(VK_FORMAT_R16_SINT);
			break;
		case vtek::ImagePixelStorageFormat::sfloat:
			priorities.push_back(VK_FORMAT_R16_SFLOAT);
			break;
		default:
			break;
		}
	}
	else if (info->channelSize == vtek::ImageChannelSize::channel_32)
	{
		switch (info->storageFormat)
		{
		case vtek::ImagePixelStorageFormat::uint:
			priorities.push_back(VK_FORMAT_R32_UINT);
			break;
		case vtek::ImagePixelStorageFormat::sint:
			priorities.push_back(VK_FORMAT_R32_SINT);
			break;
		case vtek::ImagePixelStorageFormat::sfloat:
			priorities.push_back(VK_FORMAT_R32_SFLOAT);
			break;
		default:
			break;
		}
	}
	else if (info->channelSize == vtek::ImageChannelSize::channel_64)
	{
		switch (info->storageFormat)
		{
		case vtek::ImagePixelStorageFormat::uint:
			priorities.push_back(VK_FORMAT_R64_UINT);
			break;
		case vtek::ImagePixelStorageFormat::sint:
			priorities.push_back(VK_FORMAT_R64_SINT);
			break;
		case vtek::ImagePixelStorageFormat::sfloat:
			priorities.push_back(VK_FORMAT_R64_SFLOAT);
			break;
		default:
			break;
		}
	}
	else if (info->channelSize == vtek::ImageChannelSize::special)
	{
		if (info->storageFormat == vtek::ImagePixelStorageFormat::unorm_pack16)
		{
			priorities.push_back(VK_FORMAT_R12X4_UNORM_PACK16);
			priorities.push_back(VK_FORMAT_R10X6_UNORM_PACK16);
		}
	}
}

/* implementation */
VkFormat vtek::get_format_color(
	const vtek::ImageFormatInfo* info, VkPhysicalDevice physDev,
	VkFormatFeatureFlags featureFlags)
{
	VkFormat outFormat = VK_FORMAT_UNDEFINED;
	std::vector<VkFormat> priorities;
	uint32_t channels = static_cast<uint32_t>(info->channels);

	if (channels == 1)
	{
		vtek::get_format_color_channel_1(info, priorities);
	}

	else if (channels == 2)
	{

	}

	else if (channels == 3)
	{

	}

	else if (channels == 4)
	{

	}

	priorities.push_back();
	priorities.push_back();
	priorities.push_back();
	priorities.push_back();
	priorities.push_back();
	priorities.push_back();
	priorities.push_back();
	priorities.push_back();
	priorities.push_back();
	priorities.push_back();
	priorities.push_back();
	priorities.push_back();
	priorities.push_back();
	priorities.push_back();
	priorities.push_back();
	priorities.push_back();
	priorities.push_back();
	priorities.push_back();
	priorities.push_back();

	bool find = find_supported_image_format(
		physDev, priorities, VK_IMAGE_TILING_OPTIMAL, featureFlags, outFormat);
	return find ? outFormat : VK_FORMAT_UNDEFINED;
}


VkFormat vtek::get_format_color_srgb(
	const vtek::ImageFormatInfo* info, VkPhysicalDevice physDev,
	VkFormatFeatureFlags featureFlags)
{
	VkFormat outFormat = VK_FORMAT_UNDEFINED;
	std::vector<VkFormat> priorities;
	uint32_t channels = static_cast<uint32_t>(info->channels);

	if (channels == 1)
	{
		priorities.push_back(VK_FORMAT_R8_SRGB);
	}

	else if (channels == 2)
	{
		priorities.push_back(VK_FORMAT_R8G8_SRGB);
	}

	else if (channels == 3)
	{
		if (info->compression == vtek::ImageCompressionFormat::none)
		{
			if (info->swizzleBGR)
			{
				priorities.push_back(VK_FORMAT_B8G8R8_SRGB);
			}
			else
			{
				priorities.push_back(VK_FORMAT_R8G8B8_SRGB);
			}
		}
		else if (info->compression == vtek::ImageCompressionFormat::bc)
		{
			// REVIEW: Just assuming BC7 > BC6H > BC5 > ...
			// REVIEW: This is very naive, and should probably be tweaked!

			// TODO: Are these REALLY with 3 channels???
			priorities.push_back(VK_FORMAT_BC7_SRGB_BLOCK);
			priorities.push_back(VK_FORMAT_BC3_SRGB_BLOCK);
			priorities.push_back(VK_FORMAT_BC2_SRGB_BLOCK);
			priorities.push_back(VK_FORMAT_BC1_RGB_SRGB_BLOCK);
		}
		else if (info->compression == vtek::ImageCompressionFormat::etc2)
		{
			if (!(info->swizzleBGR))
			{
				priorities.push_back(VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK);
			}
		}
		else if (info->compression == vtek::ImageCompressionFormat::astc)
		{
			// REVIEW: Undecided which is which. These are all ASTC sRGB formats:
			// VK_FORMAT_ASTC_4x4_SRGB_BLOCK = 158,
			// VK_FORMAT_ASTC_5x4_SRGB_BLOCK = 160,
			// VK_FORMAT_ASTC_5x5_SRGB_BLOCK = 162,
			// VK_FORMAT_ASTC_6x5_SRGB_BLOCK = 164,
			// VK_FORMAT_ASTC_6x6_SRGB_BLOCK = 166,
			// VK_FORMAT_ASTC_8x5_SRGB_BLOCK = 168,
			// VK_FORMAT_ASTC_8x6_SRGB_BLOCK = 170,
			// VK_FORMAT_ASTC_8x8_SRGB_BLOCK = 172,
			// VK_FORMAT_ASTC_10x5_SRGB_BLOCK = 174,
			// VK_FORMAT_ASTC_10x6_SRGB_BLOCK = 176,
			// VK_FORMAT_ASTC_10x8_SRGB_BLOCK = 178,
			// VK_FORMAT_ASTC_10x10_SRGB_BLOCK = 180,
			// VK_FORMAT_ASTC_12x10_SRGB_BLOCK = 182,
			// VK_FORMAT_ASTC_12x12_SRGB_BLOCK = 184,

			vtek_log_error("ASTC compression is not implemented!");
			return VK_FORMAT_UNDEFINED;
		}
	}

	else if (channels == 4)
	{
		if (info->compression == vtek::ImageCompressionFormat::none)
		{
			if (info->swizzleBGR &&
			    info->storageFormat == vtek::ImagePixelStorageFormat::unorm_pack32)
			{
				priorities.push_back(VK_FORMAT_A8B8G8R8_SRGB_PACK32);
			}
			else if (info->swizzleBGR)
			{
				priorities.push_back(VK_FORMAT_B8G8R8A8_SRGB);
			}
			else
			{
				priorities.push_back(VK_FORMAT_R8G8B8A8_SRGB);
			}
		}
		else if (info->compression == vtek::ImageCompressionFormat::bc)
		{
			// TODO: Check compressed formats!
			priorities.push_back(VK_FORMAT_BC1_RGBA_SRGB_BLOCK);
		}
		else if (info->compression == vtek::ImageCompressionFormat::etc2)
		{
			priorities.push_back(VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK);
			priorities.push_back(VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK);
		}
		else if (info->compression == vtek::ImageCompressionFormat::astc)
		{
			// REVIEW: Same as above

			vtek_log_error("ASTC compression is not implemented!");
			return VK_FORMAT_UNDEFINED;
		}
	}

	bool find = find_supported_image_format(
		physDev, priorities, VK_IMAGE_TILING_OPTIMAL, featureFlags, outFormat);
	return find ? outFormat : VK_FORMAT_UNDEFINED;
}
