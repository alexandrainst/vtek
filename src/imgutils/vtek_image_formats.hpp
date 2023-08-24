#pragma once

#include <cstdint>
#include <vulkan/vulkan.h>

#include "vtek_image.hpp"
#include "vtek_types.hpp"


namespace vtek
{
	// ====================== //
	// === Format queries === //
	// ====================== //

	enum class FormatFeature : uint32_t
	{
		sampledImage             = VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT,
		storageImage             = VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT,
		storageImageAtomic       = VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT,
		uniformTexelBuffer       = VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT,
		storageTexelBuffer       = VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT,
		storageTexelBufferAtomic = VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT,
		vertexBuffer             = VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT,
		colorAttachment          = VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT,
		colorAttachmentBlend     = VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT,
		depthStencilAttachment   = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT,
		blitSrc                  = VK_FORMAT_FEATURE_BLIT_SRC_BIT,
		blitDst                  = VK_FORMAT_FEATURE_BLIT_DST_BIT,
		sampledImageFilterLinear = VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT,

		// Provided by VK_VERSION_1_1
		transferSrc              = VK_FORMAT_FEATURE_TRANSFER_SRC_BIT,
		transferDst              = VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
		midpointChromaSamples    = VK_FORMAT_FEATURE_MIDPOINT_CHROMA_SAMPLES_BIT,
		sampledImageYCbCrConvLinFilter
			= VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT,
		sampledImageYCbCrConvSepRecFilter
			= VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT,
		sampledImageYCbCrConvChromaRecEx
			= VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_BIT,
		sampledImageYCbCrConvChromaRecExForce
			= VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_FORCEABLE_BIT,
		disjointBit              = VK_FORMAT_FEATURE_DISJOINT_BIT,
		cositedChromaSamples     = VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT,

		// Provided by VK_VERSION_1_2
		sampledImageFilterMinmax = VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_MINMAX_BIT
	};

	// Find first image format in the list that is supported by the
	// physical device. Returns a boolean indicating success.
	bool find_supported_image_format(
		VkPhysicalDevice physicalDevice,
		std::vector<VkFormat> prioritizedCandidates,
		VkImageTiling tiling,
		EnumBitmask<FormatFeature> featureFlags,
		VkFormat* outFormat);

	// Find a suitable format matching provided parameters.
	// This function returns `VK_FORMAT_UNDEFINED` in case a match wasn't found.
	// TODO: Better return bool instead?
	VkFormat get_format_color(
		const ImageFormatInfo* info, VkPhysicalDevice physDev,
		EnumBitmask<FormatFeature> featureFlags);

	enum class FormatDepthStencilTest
	{
		none, depth, stencil, depth_and_stencil
	};

	FormatDepthStencilTest get_format_depth_stencil_test(VkFormat format);
}
