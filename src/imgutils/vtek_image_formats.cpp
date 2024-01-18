#include "vtek_vulkan.pch"
#include "vtek_image_formats.hpp"

#include "vtek_device.hpp"
#include "vtek_format_support.hpp"
#include "vtek_logging.hpp"


/* interface */
VkFormatFeatureFlags vtek::get_format_features(
	vtek::EnumBitmask<vtek::FormatFeature> features)
{
	VkFormatFeatureFlags flags = 0;

	if (features.has_flag(vtek::FormatFeature::sampled_image)) {
		flags |= VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT;
	}
	if (features.has_flag(vtek::FormatFeature::storage_image)) {
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
