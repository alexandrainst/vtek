#pragma once

#include <vulkan/vulkan.h>

#include "vtek_object_handles.hpp"
#include "vtek_types.hpp"


namespace vtek
{
	enum class SamplerAddressMode
	{
		repeat,
		mirrored_repeat,
		clamp_to_edge,
		clamp_to_border,
		// Provided by Vulkan >= 1.2
		// NOTE: Also requires `samplerMirrorClampToEdge` feature to be
		// enabled during device creation (VkPhysicalDeviceVulkan12Features).
		mirror_clamp_to_edge
	};

	enum class SamplerBorderColor
	{
		transparent_black_float,
		transparent_black_int,
		opaque_black_float,
		opaque_black_int,
		opaque_white_float,
		opaque_white_int
	};

	enum class SamplerFilterMode { nearest, linear };

	enum class SamplerDepthCompareOp
	{
		never, // always false
		less,
		equal,
		less_or_equal,
		greater,
		not_equal,
		greater_or_equal,
		always // always true
	};

	struct SamplerInfo
	{
		SamplerAddressMode addressMode {SamplerAddressMode::repeat};
		// If address mode is `clamp_to_border`, a border color may be
		// specified from selected list.
		SamplerBorderColor borderColor {SamplerBorderColor::opaque_black_float};

		// TODO: Document.
		bool anisotropicFiltering {false};
		FloatClamp<0.0f, 16.0f> maxAnisotropy {0.0f};

		// Filtering mode for minification, magnification, and mipmap lookups.
		SamplerFilterMode minFilter {SamplerFilterMode::nearest};
		SamplerFilterMode magFilter {SamplerFilterMode::nearest};
		SamplerFilterMode mipmapFilter {SamplerFilterMode::nearest};

		// NOTE: This is only relevant for images with a depth/stencil format!
		// Specify a comparison operation to apply to fetched data before
		// filtering. This decision is based on 3 criteria:
		// 1) The image format has a depth component
		// 2) The image view applied to the image has a depth aspect flag
		// 3) The instruction is an `OpImage*Dref*`, which is a SPIR-V
		// instruction that applies a depth comparison on the texel values.
		SamplerDepthCompareOp depthCompareOp {SamplerDepthCompareOp::never};
	};


	Sampler* sampler_create(const SamplerInfo* info, Device* device);
	void sampler_destroy(Sampler* sampler, Device* device);

	VkSampler sampler_get_handle(const Sampler* sampler);
}
