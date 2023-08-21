#include "vtek_vulkan.pch"
#include "vtek_image_formats.hpp"

#include "vtek_device.hpp"
#include "vtek_format_support.hpp"
#include "vtek_logging.hpp"


/* helper functions */
static void get_format_color_srgb(
	const vtek::ImageFormatInfo* info, std::vector<VkFormat>& priorities)
{
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
		vtek_log_debug("Load sRGB, 3 channels");
		if (info->compression == vtek::ImageCompressionFormat::none)
		{
			vtek_log_debug("No compression");
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
		else if (info->compression == vtek::ImageCompressionFormat::eac)
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
			return;
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
		else if (info->compression == vtek::ImageCompressionFormat::eac)
		{
			priorities.push_back(VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK);
			priorities.push_back(VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK);
		}
		else if (info->compression == vtek::ImageCompressionFormat::astc)
		{
			// REVIEW: Same as above

			vtek_log_error("ASTC compression is not implemented!");
			return;
		}
	}
}



static void get_format_color_channel_1(
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


static void get_format_color_channel_2(
	const vtek::ImageFormatInfo* info, std::vector<VkFormat>& priorities)
{
	if (info->channelSize == vtek::ImageChannelSize::channel_8)
	{
		switch (info->storageFormat)
		{
		case vtek::ImagePixelStorageFormat::unorm:
			priorities.push_back(VK_FORMAT_R8G8_UNORM);
			break;
		case vtek::ImagePixelStorageFormat::snorm:
			priorities.push_back(VK_FORMAT_R8G8_SNORM);
			break;
		case vtek::ImagePixelStorageFormat::uscaled:
			priorities.push_back(VK_FORMAT_R8G8_USCALED);
			break;
		case vtek::ImagePixelStorageFormat::sscaled:
			priorities.push_back(VK_FORMAT_R8G8_SSCALED);
			break;
		case vtek::ImagePixelStorageFormat::uint:
			priorities.push_back(VK_FORMAT_R8G8_UINT);
			break;
		case vtek::ImagePixelStorageFormat::sint:
			priorities.push_back(VK_FORMAT_R8G8_SINT);
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
			priorities.push_back(VK_FORMAT_R16G16_UNORM);
			break;
		case vtek::ImagePixelStorageFormat::snorm:
			priorities.push_back(VK_FORMAT_R16G16_SNORM);
			break;
		case vtek::ImagePixelStorageFormat::uscaled:
			priorities.push_back(VK_FORMAT_R16G16_USCALED);
			break;
		case vtek::ImagePixelStorageFormat::sscaled:
			priorities.push_back(VK_FORMAT_R16G16_SSCALED);
			break;
		case vtek::ImagePixelStorageFormat::uint:
			priorities.push_back(VK_FORMAT_R16G16_UINT);
			break;
		case vtek::ImagePixelStorageFormat::sint:
			priorities.push_back(VK_FORMAT_R16G16_SINT);
			break;
		case vtek::ImagePixelStorageFormat::sfloat:
			priorities.push_back(VK_FORMAT_R16G16_SFLOAT);
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
			priorities.push_back(VK_FORMAT_R32G32_UINT);
			break;
		case vtek::ImagePixelStorageFormat::sint:
			priorities.push_back(VK_FORMAT_R32G32_SINT);
			break;
		case vtek::ImagePixelStorageFormat::sfloat:
			priorities.push_back(VK_FORMAT_R32G32_SFLOAT);
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
			priorities.push_back(VK_FORMAT_R64G64_UINT);
			break;
		case vtek::ImagePixelStorageFormat::sint:
			priorities.push_back(VK_FORMAT_R64G64_SINT);
			break;
		case vtek::ImagePixelStorageFormat::sfloat:
			priorities.push_back(VK_FORMAT_R64G64_SFLOAT);
			break;
		default:
			break;
		}
	}
	else if (info->channelSize == vtek::ImageChannelSize::special)
	{
		if (info->storageFormat == vtek::ImagePixelStorageFormat::unorm_pack8)
		{
			priorities.push_back(VK_FORMAT_R4G4_UNORM_PACK8);
		}
		else if (info->storageFormat == vtek::ImagePixelStorageFormat::unorm_pack16)
		{
			priorities.push_back(VK_FORMAT_R12X4G12X4_UNORM_2PACK16);
			priorities.push_back(VK_FORMAT_R10X6G10X6_UNORM_2PACK16);
		}
		else if (info->compression == vtek::ImageCompressionFormat::eac)
		{
			switch (info->storageFormat)
			{
			case vtek::ImagePixelStorageFormat::unorm:
				priorities.push_back(VK_FORMAT_EAC_R11G11_UNORM_BLOCK);
				break;
			case vtek::ImagePixelStorageFormat::snorm:
				priorities.push_back(VK_FORMAT_EAC_R11G11_SNORM_BLOCK);
				break;
			default:
				break;
			}
		}
	}
}



/* initialization */
vtek::FormatSupport::FormatSupport(const vtek::Device* device)
{
	VkPhysicalDevice physDev = vtek::device_get_physical_handle(device);
	std::vector<VkFormat> priorities;
	vtek::EnumBitmask<vtek::FormatFeatureFlag> featureFlags
		= vtek::FormatFeatureFlag::sampledImage
		| vtek::FormatFeatureFlag::sampledImageFilterLinear;
	VkFormat outFormat; // ignored here

	priorities.push_back(VK_FORMAT_R8_SRGB);
	if (vtek::find_supported_image_format(
		    physDev, priorities, VK_IMAGE_TILING_OPTIMAL, featureFlags, &outFormat))
	{
		mSRGB |= 0x01;
	}
	priorities.clear();
}



/* interface */
bool vtek::find_supported_image_format(
	VkPhysicalDevice physicalDevice, std::vector<VkFormat> prioritizedCandidates,
	VkImageTiling tiling, EnumBitmask<FormatFeatureFlag> featureFlags, VkFormat* outFormat)
{
	VkFormatFeatureFlags features = featureFlags.get();
	for (const VkFormat& format : prioritizedCandidates)
	{
		VkFormatProperties properties;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &properties);

		if (tiling == VK_IMAGE_TILING_LINEAR &&
		    (properties.linearTilingFeatures & features) == features)
		{
			vtek_log_debug("LINEAR and features");
			*outFormat = format;
			return true;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
		         (properties.optimalTilingFeatures & features) == features)
		{
			vtek_log_debug("OPTIMAL and features");
			*outFormat = format;
			return true;
		}
		else
		{
			vtek_log_debug("else");
		}
	}

	return false;
}



VkFormat vtek::get_format_color(
	const vtek::ImageFormatInfo* info, VkPhysicalDevice physDev,
	vtek::EnumBitmask<vtek::FormatFeatureFlag> featureFlags)
{
	VkFormat outFormat = VK_FORMAT_UNDEFINED;
	std::vector<VkFormat> priorities;
	uint32_t channels = static_cast<uint32_t>(info->channels);

	if (info->imageStorageSRGB)
	{
		get_format_color_srgb(info, priorities);
	}
	else if (channels == 1)
	{
		get_format_color_channel_1(info, priorities);
	}
	else if (channels == 2)
	{
		get_format_color_channel_2(info, priorities);
	}
	else if (channels == 3)
	{
		vtek_log_debug("get_format_color 3 channels: Not implemented!");
	}
	else if (channels == 4)
	{
		vtek_log_debug("get_format_color 4 channels: Not implemented!");
	}

	vtek_log_debug("Found {} possible format matches", priorities.size());

	bool find = find_supported_image_format(
		physDev, priorities, VK_IMAGE_TILING_OPTIMAL, featureFlags, &outFormat);
	return find ? outFormat : VK_FORMAT_UNDEFINED;
}
