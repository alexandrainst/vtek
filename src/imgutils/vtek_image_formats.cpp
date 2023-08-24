#include "vtek_vulkan.pch"
#include "vtek_image_formats.hpp"

#include "vtek_device.hpp"
#include "vtek_format_support.hpp"
#include "vtek_logging.hpp"


/* helper functions */
static void get_format_color_srgb(
	const vtek::FormatInfo* info, std::vector<VkFormat>& priorities)
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
		if (info->swizzleBGR)
		{
			priorities.push_back(VK_FORMAT_B8G8R8_SRGB);
		}
		else
		{
			priorities.push_back(VK_FORMAT_R8G8B8_SRGB);
		}
	}
	else if (channels == 4)
	{
		if (info->swizzleBGR &&
		    info->storageType == vtek::FormatStorageType::unorm_pack32)
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
}



static void get_format_color_channel_1(
	const vtek::FormatInfo* info, std::vector<VkFormat>& priorities)
{
	if (info->channelSize == vtek::FormatChannelSize::channel_8)
	{
		switch (info->storageType)
		{
		case vtek::FormatStorageType::unorm:
			priorities.push_back(VK_FORMAT_R8_UNORM);
			break;
		case vtek::FormatStorageType::snorm:
			priorities.push_back(VK_FORMAT_R8_SNORM);
			break;
		case vtek::FormatStorageType::uscaled:
			priorities.push_back(VK_FORMAT_R8_USCALED);
			break;
		case vtek::FormatStorageType::sscaled:
			priorities.push_back(VK_FORMAT_R8_SSCALED);
			break;
		case vtek::FormatStorageType::uint:
			priorities.push_back(VK_FORMAT_R8_UINT);
			break;
		case vtek::FormatStorageType::sint:
			priorities.push_back(VK_FORMAT_R8_SINT);
			break;
		default:
			break;
		}
	}
	else if (info->channelSize == vtek::FormatChannelSize::channel_16)
	{
		switch (info->storageType)
		{
		case vtek::FormatStorageType::unorm:
			priorities.push_back(VK_FORMAT_R16_UNORM);
			break;
		case vtek::FormatStorageType::snorm:
			priorities.push_back(VK_FORMAT_R16_SNORM);
			break;
		case vtek::FormatStorageType::uscaled:
			priorities.push_back(VK_FORMAT_R16_USCALED);
			break;
		case vtek::FormatStorageType::sscaled:
			priorities.push_back(VK_FORMAT_R16_SSCALED);
			break;
		case vtek::FormatStorageType::uint:
			priorities.push_back(VK_FORMAT_R16_UINT);
			break;
		case vtek::FormatStorageType::sint:
			priorities.push_back(VK_FORMAT_R16_SINT);
			break;
		case vtek::FormatStorageType::sfloat:
			priorities.push_back(VK_FORMAT_R16_SFLOAT);
			break;
		default:
			break;
		}
	}
	else if (info->channelSize == vtek::FormatChannelSize::channel_32)
	{
		switch (info->storageType)
		{
		case vtek::FormatStorageType::uint:
			priorities.push_back(VK_FORMAT_R32_UINT);
			break;
		case vtek::FormatStorageType::sint:
			priorities.push_back(VK_FORMAT_R32_SINT);
			break;
		case vtek::FormatStorageType::sfloat:
			priorities.push_back(VK_FORMAT_R32_SFLOAT);
			break;
		default:
			break;
		}
	}
	else if (info->channelSize == vtek::FormatChannelSize::channel_64)
	{
		switch (info->storageType)
		{
		case vtek::FormatStorageType::uint:
			priorities.push_back(VK_FORMAT_R64_UINT);
			break;
		case vtek::FormatStorageType::sint:
			priorities.push_back(VK_FORMAT_R64_SINT);
			break;
		case vtek::FormatStorageType::sfloat:
			priorities.push_back(VK_FORMAT_R64_SFLOAT);
			break;
		default:
			break;
		}
	}
	else if (info->channelSize == vtek::FormatChannelSize::special)
	{
		if (info->storageType == vtek::FormatStorageType::unorm_pack16)
		{
			priorities.push_back(VK_FORMAT_R12X4_UNORM_PACK16);
			priorities.push_back(VK_FORMAT_R10X6_UNORM_PACK16);
		}
	}
}


static void get_format_color_channel_2(
	const vtek::FormatInfo* info, std::vector<VkFormat>& priorities)
{
	if (info->channelSize == vtek::FormatChannelSize::channel_8)
	{
		switch (info->storageType)
		{
		case vtek::FormatStorageType::unorm:
			priorities.push_back(VK_FORMAT_R8G8_UNORM);
			break;
		case vtek::FormatStorageType::snorm:
			priorities.push_back(VK_FORMAT_R8G8_SNORM);
			break;
		case vtek::FormatStorageType::uscaled:
			priorities.push_back(VK_FORMAT_R8G8_USCALED);
			break;
		case vtek::FormatStorageType::sscaled:
			priorities.push_back(VK_FORMAT_R8G8_SSCALED);
			break;
		case vtek::FormatStorageType::uint:
			priorities.push_back(VK_FORMAT_R8G8_UINT);
			break;
		case vtek::FormatStorageType::sint:
			priorities.push_back(VK_FORMAT_R8G8_SINT);
			break;
		default:
			break;
		}
	}
	else if (info->channelSize == vtek::FormatChannelSize::channel_16)
	{
		switch (info->storageType)
		{
		case vtek::FormatStorageType::unorm:
			priorities.push_back(VK_FORMAT_R16G16_UNORM);
			break;
		case vtek::FormatStorageType::snorm:
			priorities.push_back(VK_FORMAT_R16G16_SNORM);
			break;
		case vtek::FormatStorageType::uscaled:
			priorities.push_back(VK_FORMAT_R16G16_USCALED);
			break;
		case vtek::FormatStorageType::sscaled:
			priorities.push_back(VK_FORMAT_R16G16_SSCALED);
			break;
		case vtek::FormatStorageType::uint:
			priorities.push_back(VK_FORMAT_R16G16_UINT);
			break;
		case vtek::FormatStorageType::sint:
			priorities.push_back(VK_FORMAT_R16G16_SINT);
			break;
		case vtek::FormatStorageType::sfloat:
			priorities.push_back(VK_FORMAT_R16G16_SFLOAT);
			break;
		default:
			break;
		}
	}
	else if (info->channelSize == vtek::FormatChannelSize::channel_32)
	{
		switch (info->storageType)
		{
		case vtek::FormatStorageType::uint:
			priorities.push_back(VK_FORMAT_R32G32_UINT);
			break;
		case vtek::FormatStorageType::sint:
			priorities.push_back(VK_FORMAT_R32G32_SINT);
			break;
		case vtek::FormatStorageType::sfloat:
			priorities.push_back(VK_FORMAT_R32G32_SFLOAT);
			break;
		default:
			break;
		}
	}
	else if (info->channelSize == vtek::FormatChannelSize::channel_64)
	{
		switch (info->storageType)
		{
		case vtek::FormatStorageType::uint:
			priorities.push_back(VK_FORMAT_R64G64_UINT);
			break;
		case vtek::FormatStorageType::sint:
			priorities.push_back(VK_FORMAT_R64G64_SINT);
			break;
		case vtek::FormatStorageType::sfloat:
			priorities.push_back(VK_FORMAT_R64G64_SFLOAT);
			break;
		default:
			break;
		}
	}
	else if (info->channelSize == vtek::FormatChannelSize::special)
	{
		if (info->storageType == vtek::FormatStorageType::unorm_pack8)
		{
			priorities.push_back(VK_FORMAT_R4G4_UNORM_PACK8);
		}
		else if (info->storageType == vtek::FormatStorageType::unorm_pack16)
		{
			priorities.push_back(VK_FORMAT_R12X4G12X4_UNORM_2PACK16);
			priorities.push_back(VK_FORMAT_R10X6G10X6_UNORM_2PACK16);
		}
	}
}

static void get_format_color_channel_3(
	const vtek::FormatInfo* info, std::vector<VkFormat>& priorities)
{
	if (info->channelSize == vtek::FormatChannelSize::channel_8)
	{
		switch (info->storageType)
		{
		case vtek::FormatStorageType::unorm:
			priorities.push_back(
				(info->swizzleBGR)
				? VK_FORMAT_B8G8R8_UNORM : VK_FORMAT_R8G8B8_UNORM);
			break;
		case vtek::FormatStorageType::snorm:
			priorities.push_back(
				(info->swizzleBGR)
				? VK_FORMAT_B8G8R8_SNORM : VK_FORMAT_R8G8B8_SNORM);
			break;
		case vtek::FormatStorageType::uscaled:
			priorities.push_back(
				(info->swizzleBGR)
				? VK_FORMAT_B8G8R8_USCALED : VK_FORMAT_R8G8B8_USCALED);
			break;
		case vtek::FormatStorageType::sscaled:
			priorities.push_back(
				(info->swizzleBGR)
				? VK_FORMAT_B8G8R8_SSCALED : VK_FORMAT_R8G8B8_SSCALED);
			break;
		case vtek::FormatStorageType::uint:
			priorities.push_back(
				(info->swizzleBGR)
				? VK_FORMAT_B8G8R8_UINT : VK_FORMAT_R8G8B8_UINT);
			break;
		case vtek::FormatStorageType::sint:
			priorities.push_back(
				(info->swizzleBGR)
				? VK_FORMAT_B8G8R8_SINT : VK_FORMAT_R8G8B8_SINT);
			break;
		default:
			break;
		}
	}
	else if (info->channelSize == vtek::FormatChannelSize::channel_16 &&
	         !info->swizzleBGR)
	{
		switch (info->storageType)
		{
		case vtek::FormatStorageType::unorm:
			priorities.push_back(VK_FORMAT_R16G16B16_UNORM);
			break;
		case vtek::FormatStorageType::snorm:
			priorities.push_back(VK_FORMAT_R16G16B16_SNORM);
			break;
		case vtek::FormatStorageType::uscaled:
			priorities.push_back(VK_FORMAT_R16G16B16_USCALED);
			break;
		case vtek::FormatStorageType::sscaled:
			priorities.push_back(VK_FORMAT_R16G16B16_SSCALED);
			break;
		case vtek::FormatStorageType::uint:
			priorities.push_back(VK_FORMAT_R16G16B16_UINT);
			break;
		case vtek::FormatStorageType::sint:
			priorities.push_back(VK_FORMAT_R16G16B16_SINT);
			break;
		case vtek::FormatStorageType::sfloat:
			priorities.push_back(VK_FORMAT_R16G16B16_SFLOAT);
			break;
		default:
			break;
		}
	}
	else if (info->channelSize == vtek::FormatChannelSize::channel_32 &&
	         !info->swizzleBGR)
	{
		switch (info->storageType)
		{
		case vtek::FormatStorageType::uint:
			priorities.push_back(VK_FORMAT_R32G32B32_UINT);
			break;
		case vtek::FormatStorageType::sint:
			priorities.push_back(VK_FORMAT_R32G32B32_SINT);
			break;
		case vtek::FormatStorageType::sfloat:
			priorities.push_back(VK_FORMAT_R32G32B32_SFLOAT);
			break;
		default:
			break;
		}
	}
	else if (info->channelSize == vtek::FormatChannelSize::channel_64 &&
	         !info->swizzleBGR)
	{
		switch (info->storageType)
		{
		case vtek::FormatStorageType::uint:
			priorities.push_back(VK_FORMAT_R64G64B64_UINT);
			break;
		case vtek::FormatStorageType::sint:
			priorities.push_back(VK_FORMAT_R64G64B64_SINT);
			break;
		case vtek::FormatStorageType::sfloat:
			priorities.push_back(VK_FORMAT_R64G64B64_SFLOAT);
			break;
		default:
			break;
		}
	}
	else if (info->channelSize == vtek::FormatChannelSize::special)
	{
		if (info->storageType == vtek::FormatStorageType::ufloat_pack32)
		{
			priorities.push_back(VK_FORMAT_B10G11R11_UFLOAT_PACK32);
		}
	}
}

static void get_format_color_channel_4(
	const vtek::FormatInfo* info, std::vector<VkFormat>& priorities)
{
	if (info->channelSize == vtek::FormatChannelSize::channel_8)
	{
		switch (info->storageType)
		{
		case vtek::FormatStorageType::unorm:
			priorities.push_back(
				(info->swizzleBGR)
				? VK_FORMAT_B8G8R8A8_UNORM : VK_FORMAT_R8G8B8A8_UNORM);
			break;
		case vtek::FormatStorageType::snorm:
			priorities.push_back(
				(info->swizzleBGR)
				? VK_FORMAT_B8G8R8A8_SNORM : VK_FORMAT_R8G8B8A8_SNORM);
			break;
		case vtek::FormatStorageType::uscaled:
			priorities.push_back(
				(info->swizzleBGR)
				? VK_FORMAT_B8G8R8A8_USCALED : VK_FORMAT_R8G8B8A8_USCALED);
			break;
		case vtek::FormatStorageType::sscaled:
			priorities.push_back(
				(info->swizzleBGR)
				? VK_FORMAT_B8G8R8A8_SSCALED : VK_FORMAT_R8G8B8A8_SSCALED);
			break;
		case vtek::FormatStorageType::uint:
			priorities.push_back(
				(info->swizzleBGR)
				? VK_FORMAT_B8G8R8A8_UINT : VK_FORMAT_R8G8B8A8_UINT);
			break;
		case vtek::FormatStorageType::sint:
			priorities.push_back(
				(info->swizzleBGR)
				? VK_FORMAT_B8G8R8A8_SINT : VK_FORMAT_R8G8B8A8_SINT);
			break;
		case vtek::FormatStorageType::unorm_pack32:
			priorities.push_back(VK_FORMAT_A8B8G8R8_UNORM_PACK32);
			break;
		case vtek::FormatStorageType::snorm_pack32:
			priorities.push_back(VK_FORMAT_A8B8G8R8_SNORM_PACK32);
			break;
		case vtek::FormatStorageType::uscaled_pack32:
			priorities.push_back(VK_FORMAT_A8B8G8R8_USCALED_PACK32);
			break;
		case vtek::FormatStorageType::sscaled_pack32:
			priorities.push_back(VK_FORMAT_A8B8G8R8_SSCALED_PACK32);
			break;
		case vtek::FormatStorageType::uint_pack32:
			priorities.push_back(VK_FORMAT_A8B8G8R8_UINT_PACK32);
			break;
		case vtek::FormatStorageType::sint_pack32:
			priorities.push_back(VK_FORMAT_A8B8G8R8_SINT_PACK32);
			break;
		default:
			break;
		}
	}
	else if (info->channelSize == vtek::FormatChannelSize::channel_16)
	{
		switch (info->storageType)
		{
		case vtek::FormatStorageType::unorm:
			priorities.push_back(VK_FORMAT_R16G16B16A16_UNORM);
			break;
		case vtek::FormatStorageType::snorm:
			priorities.push_back(VK_FORMAT_R16G16B16A16_SNORM);
			break;
		case vtek::FormatStorageType::uscaled:
			priorities.push_back(VK_FORMAT_R16G16B16A16_USCALED);
			break;
		case vtek::FormatStorageType::sscaled:
			priorities.push_back(VK_FORMAT_R16G16B16A16_SSCALED);
			break;
		case vtek::FormatStorageType::uint:
			priorities.push_back(VK_FORMAT_R16G16B16A16_UINT);
			break;
		case vtek::FormatStorageType::sint:
			priorities.push_back(VK_FORMAT_R16G16B16A16_SINT);
			break;
		case vtek::FormatStorageType::sfloat:
			priorities.push_back(VK_FORMAT_R16G16B16A16_SFLOAT);
			break;
		default:
			break;
		}
	}
	else if (info->channelSize == vtek::FormatChannelSize::channel_32)
	{
		switch (info->storageType)
		{
		case vtek::FormatStorageType::uint:
			priorities.push_back(VK_FORMAT_R32G32B32A32_UINT);
			break;
		case vtek::FormatStorageType::sint:
			priorities.push_back(VK_FORMAT_R32G32B32A32_SINT);
			break;
		case vtek::FormatStorageType::sfloat:
			priorities.push_back(VK_FORMAT_R32G32B32A32_SFLOAT);
			break;
		default:
			break;
		}
	}
	else if (info->channelSize == vtek::FormatChannelSize::channel_64)
	{
		switch (info->storageType)
		{
		case vtek::FormatStorageType::uint:
			priorities.push_back(VK_FORMAT_R64G64B64A64_UINT);
			break;
		case vtek::FormatStorageType::sint:
			priorities.push_back(VK_FORMAT_R64G64B64A64_SINT);
			break;
		case vtek::FormatStorageType::sfloat:
			priorities.push_back(VK_FORMAT_R64G64B64A64_SFLOAT);
			break;
		default:
			break;
		}
	}
	else if (info->channelSize == vtek::FormatChannelSize::special &&
	         !info->swizzleBGR)
	{
		switch (info->storageType)
		{
		case vtek::FormatStorageType::unorm_pack16:
			priorities.push_back(VK_FORMAT_R4G4B4A4_UNORM_PACK16);
#if defined(VK_API_VERSION_1_3)
			priorities.push_back(VK_FORMAT_A4R4G4B4_UNORM_PACK16);
#endif
			priorities.push_back(VK_FORMAT_R5G5B5A1_UNORM_PACK16);
			priorities.push_back(VK_FORMAT_A1R5G5B5_UNORM_PACK16);
			break;
		case vtek::FormatStorageType::unorm_pack32:
			priorities.push_back(VK_FORMAT_A2R10G10B10_UNORM_PACK32);
			break;
		case vtek::FormatStorageType::snorm_pack32:
			priorities.push_back(VK_FORMAT_A2R10G10B10_SNORM_PACK32);
			break;
		case vtek::FormatStorageType::uscaled_pack32:
			priorities.push_back(VK_FORMAT_A2R10G10B10_USCALED_PACK32);
			break;
		case vtek::FormatStorageType::sscaled_pack32:
			priorities.push_back(VK_FORMAT_A2R10G10B10_SSCALED_PACK32);
			break;
		case vtek::FormatStorageType::uint_pack32:
			priorities.push_back(VK_FORMAT_A2R10G10B10_UINT_PACK32);
			break;
		case vtek::FormatStorageType::sint_pack32:
			priorities.push_back(VK_FORMAT_A2R10G10B10_SINT_PACK32);
			break;
		default:
			break;
		}
	}
	else if (info->channelSize == vtek::FormatChannelSize::special &&
	         info->swizzleBGR)
	{
		switch (info->storageType)
		{
		case vtek::FormatStorageType::unorm_pack16:
			priorities.push_back(VK_FORMAT_B4G4R4A4_UNORM_PACK16);
#if defined(VK_API_VERSION_1_3)
			priorities.push_back(VK_FORMAT_A4B4G4R4_UNORM_PACK16);
#endif
			priorities.push_back(VK_FORMAT_B5G5R5A1_UNORM_PACK16);
			break;
		case vtek::FormatStorageType::unorm_pack32:
			priorities.push_back(VK_FORMAT_A2B10G10R10_UNORM_PACK32);
			break;
		case vtek::FormatStorageType::snorm_pack32:
			priorities.push_back(VK_FORMAT_A2B10G10R10_SNORM_PACK32);
			break;
		case vtek::FormatStorageType::uscaled_pack32:
			priorities.push_back(VK_FORMAT_A2B10G10R10_USCALED_PACK32);
			break;
		case vtek::FormatStorageType::sscaled_pack32:
			priorities.push_back(VK_FORMAT_A2B10G10R10_SSCALED_PACK32);
			break;
		case vtek::FormatStorageType::uint_pack32:
			priorities.push_back(VK_FORMAT_A2B10G10R10_UINT_PACK32);
			break;
		case vtek::FormatStorageType::sint_pack32:
			priorities.push_back(VK_FORMAT_A2B10G10R10_SINT_PACK32);
			break;
		case vtek::FormatStorageType::ufloat_pack32:
			priorities.push_back(VK_FORMAT_E5B9G9R9_UFLOAT_PACK32);
			break;
		default:
			break;
		}
	}
}

static void get_format_compressed_srgb(
	const vtek::FormatInfo* info, std::vector<VkFormat>& priorities)
{
	vtek_log_error("get_format_compressed_srgb(): Not implemented!");
}

static void get_format_compressed(
	const vtek::FormatInfo* info, std::vector<VkFormat>& priorities)
{
	vtek_log_error("get_format_compressed(): Not implemented!");
}



/* interface */
VkFormatFeatureFlags vtek::get_format_features(
	vtek::EnumBitmask<vtek::FormatFeature> features)
{
	VkFormatFeatureFlags flags = 0;

	if (features.has_flag(vtek::FormatFeature::sampled_image)) {
		flags |= VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT;
	}
	if (features.has_flag(vtek::FormatFeature::sampled_image)) {
		flags |= VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT;
	}
	if (features.has_flag(vtek::FormatFeature::storage_image_atomic)) {
		flags |= VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT;
	}
	if (features.has_flag(vtek::FormatFeature::uniform_texel_buffer)) {
		flags |= VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT;
	}
	if (features.has_flag(vtek::FormatFeature::storage_texel_buffer)) {
		flags |= VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT;
	}
	if (features.has_flag(vtek::FormatFeature::storage_texel_buffer_atomic)) {
		flags |= VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT;
	}
	if (features.has_flag(vtek::FormatFeature::vertex_buffer)) {
		flags |= VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT;
	}
	if (features.has_flag(vtek::FormatFeature::color_attachment)) {
		flags |= VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT;
	}
	if (features.has_flag(vtek::FormatFeature::color_attachment_blend)) {
		flags |= VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT;
	}
	if (features.has_flag(vtek::FormatFeature::depth_stencil_attachment)) {
		flags |= VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
	}
	if (features.has_flag(vtek::FormatFeature::blit_src)) {
		flags |= VK_FORMAT_FEATURE_BLIT_SRC_BIT;
	}
	if (features.has_flag(vtek::FormatFeature::blit_dst)) {
		flags |= VK_FORMAT_FEATURE_BLIT_DST_BIT;
	}
	if (features.has_flag(vtek::FormatFeature::sampled_image_filter_linear)) {
		flags |= VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;
	}

#if defined(VK_API_VERSION_1_1)
	if (features.has_flag(vtek::FormatFeature::transfer_src)) {
		flags |= VK_FORMAT_FEATURE_TRANSFER_SRC_BIT;
	}
	if (features.has_flag(vtek::FormatFeature::transfer_dst)) {
		flags |= VK_FORMAT_FEATURE_TRANSFER_DST_BIT;
	}
	if (features.has_flag(vtek::FormatFeature::midpoint_chroma_samples)) {
		flags |= VK_FORMAT_FEATURE_MIDPOINT_CHROMA_SAMPLES_BIT;
	}
	if (features.has_flag(vtek::FormatFeature::sampled_image_ycbcr_conv_lin_filter)) {
		flags |= VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT;
	}
	if (features.has_flag(vtek::FormatFeature::sampled_image_ycbcr_conv_sep_rec_filter)) {
		flags |= VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT;
	}
	if (features.has_flag(vtek::FormatFeature::sampled_image_ycbcr_conv_chroma_Rec_ex)) {
		flags |= VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_BIT;
	}
	if (features.has_flag(vtek::FormatFeature::sampled_image_ycbcr_conv_chroma_rec_ex_force)) {
		flags |= VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_FORCEABLE_BIT;
	}
	if (features.has_flag(vtek::FormatFeature::disjoint_bit)) {
		flags |= VK_FORMAT_FEATURE_DISJOINT_BIT;
	}
	if (features.has_flag(vtek::FormatFeature::cosited_chroma_samples)) {
		flags |= VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT;
	}
#endif

#if defined(VK_API_VERSION_1_2)
	if (features.has_flag(vtek::FormatFeature::sampled_image_filter_minmax)) {
		flags |= VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_MINMAX_BIT;
	}
#endif

	return flags;
}

bool vtek::find_supported_image_format(
	VkPhysicalDevice physicalDevice, std::vector<VkFormat> prioritizedCandidates,
	VkImageTiling tiling, vtek::EnumBitmask<FormatFeature> featureFlags,
	VkFormat* outFormat)
{
	VkFormatFeatureFlags features = vtek::get_format_features(featureFlags);
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
	const vtek::FormatInfo* info, VkPhysicalDevice physDev,
	vtek::EnumBitmask<vtek::FormatFeature> featureFlags)
{
	VkFormat outFormat = VK_FORMAT_UNDEFINED;
	std::vector<VkFormat> priorities;
	uint32_t channels = static_cast<uint32_t>(info->channels);

	// TODO: Independent function for all compressed formats!
	if (info->compression != vtek::FormatCompression::none)
	{
		if (info->sRGB)
		{
			get_format_compressed_srgb(info, priorities);
		}
		else
		{
			get_format_compressed(info, priorities);
		}
	}
	else if (info->sRGB)
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
		get_format_color_channel_3(info, priorities);
	}
	else if (channels == 4)
	{
		get_format_color_channel_4(info, priorities);
	}

	bool find = find_supported_image_format(
		physDev, priorities, VK_IMAGE_TILING_OPTIMAL, featureFlags, &outFormat);
	return find ? outFormat : VK_FORMAT_UNDEFINED;
}



vtek::FormatDepthStencilTest vtek::get_format_depth_stencil_test(VkFormat format)
{
	switch (format)
	{
	case VK_FORMAT_D16_UNORM:
	case VK_FORMAT_X8_D24_UNORM_PACK32:
	case VK_FORMAT_D32_SFLOAT:
		return vtek::FormatDepthStencilTest::depth;
	case VK_FORMAT_S8_UINT:
		return vtek::FormatDepthStencilTest::stencil;
	case VK_FORMAT_D16_UNORM_S8_UINT:
	case VK_FORMAT_D24_UNORM_S8_UINT:
	case VK_FORMAT_D32_SFLOAT_S8_UINT:
		return vtek::FormatDepthStencilTest::depth_and_stencil;
	default:
		return vtek::FormatDepthStencilTest::none;
	}
}
